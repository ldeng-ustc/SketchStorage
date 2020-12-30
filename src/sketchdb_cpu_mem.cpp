#include <cstdio>
#include <cstring>
#include <string>
#include <random>
#include <unordered_map>
#include <getopt.h>
#include <sys/resource.h>

#include "modules/flow.h"
#include "modules/sketchdb.h"
#include "modules/trace.h"
#include "modules/profiling.h"

using namespace sketchstorage;
using namespace std;

const char * Db_Path = "./data/db/test";
const char * Trace_Path = "./data/caida/trace.bin";
const char * Output_Path_Std = "./data/test/true_flows.txt";
const char * Output_Path_Db = "./data/test/db_flows.txt";
string Eval_Data_Dir = "./data/evaluations/sketchdb/profile_std.csv";
int duration_ms = 60000;
bool insert_without_index = false;
bool insert_without_switch = false;

void parse_args (int argc, char **argv)
{
    int c;
    while((c=getopt(argc, argv, "d:o:iw")) != -1) {
        switch (c)
        {
        case 'd':
            sscanf(optarg, "%d", &duration_ms);
            break;
        case 'o':
            Eval_Data_Dir = string(optarg);
            break;
        case 'i':
            insert_without_index = true;
        case 'w':
            insert_without_switch = true;
        default:
            break;
        }
    }
}

int main(int argc, char ** argv) {
    parse_args(argc, argv);

    SketchDB db(Db_Path);
    TraceIterator trace_iter(Trace_Path);
    double trace_start_time = (*trace_iter).ts;

    string Eval_Data_Path = Eval_Data_Dir;
    FILE * std_flows = fopen(Output_Path_Std, "w");
    FILE * eval_output = fopen(Eval_Data_Path.c_str(), "w");

    timeval epoch_id = GetEpochId(trace_start_time);
    timeval duration = {duration_ms / 1000, duration_ms % 1000 * 1000};
    timeval end_epoch;
    timeradd(&epoch_id, &duration, &end_epoch);

    unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_map;
    PacketInfo pkt;
    int epoch_cnt = 0;
    int flow_cnt = 0;
    int pkt_cnt = 0;
    
    Clock c;
    double vm, rss;
    int insert_time;
    process_mem_usage(&vm, &rss);
    while(trace_iter.next(&pkt)) {
        if(CmpTimeval(GetEpochId(pkt.ts), epoch_id) != 0) {
            vector<Flow> flows_list;
            for(auto flow_pair: flows_map) {
                flows_list.push_back(Flow(flow_pair));
            }
            flow_cnt += flows_map.size();
            flows_map.clear();
            for(auto flow: flows_list) {
                flow.Print(std_flows);
            }
            c.Start();
            if(insert_without_index){
                db.PutFlowset_Fake(epoch_id, flows_list);
            }
            else if(insert_without_switch){
                db.PutFlowset_WithoutSwitch(epoch_id, flows_list);
            }
            else{
                db.PutFlowset(epoch_id, flows_list);
            }
            insert_time = c.Stop();
            process_mem_usage(&vm, &rss);
            int epoch_time_ms = TimevalToLong(epoch_id) / 1000 % (60 * 1000);
            fprintf(eval_output, "%d,%d,%d\n", epoch_time_ms, static_cast<int>(rss), insert_time);
            epoch_id = GetEpochId(pkt.ts);
            epoch_cnt ++;
            if(CmpTimeval(epoch_id, end_epoch) >= 0) {
                break;
            }
        }
        flows_map[pkt.key].AddPacket(pkt);
        pkt_cnt ++;
    }
    fclose(std_flows);

    vector<Flow> result;
    epoch_id = GetEpochId(trace_start_time);
    timeval step = {0, 1000};
    while (CmpTimeval(epoch_id, end_epoch) < 0) {
        db.Get(epoch_id, &result);
        timeradd(&epoch_id, &step, &epoch_id);
    }
    FILE * db_flows = fopen(Output_Path_Db, "w");
    for(auto flow: result) {
        flow.Print(db_flows);
    }
    fclose(db_flows);

    char cmd[200];
    sprintf(cmd, "diff -q %s %s", Output_Path_Std, Output_Path_Db);
    if(system(cmd) == 0) {
        printf(ANSI_COLOR_GREEN "[Check OK!]\n" ANSI_COLOR_RESET);
    }
    else {
        printf(ANSI_COLOR_RED "[Check Failed!]\n" ANSI_COLOR_RESET);
    }
    db.Close();
    rocksdb::DestroyDB(Db_Path, rocksdb::Options());
    return 0;
}