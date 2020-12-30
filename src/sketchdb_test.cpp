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
string Eval_Data_Dir = "./data/evaluations/sketchdb/";
int duration_ms = 60000;
bool insert_without_index = false;

void parse_args (int argc, char **argv)
{
    int c;
    while((c=getopt(argc, argv, "d:o:")) != -1) {
        switch (c)
        {
        case 'd':
            sscanf(optarg, "%d", &duration_ms);
            break;
        case 'o':
            Eval_Data_Dir = string(optarg);
        default:
            break;
        }
    }
}

typedef void (*PutFlowsetFunc)(SketchDB *, const timeval &, const vector<Flow> &);

void PutFlowset(SketchDB * db, const timeval & epoch_id, 
    const vector<Flow> & flow_list ) {
    db->PutFlowset(epoch_id, flow_list);
}

void PutFlowsetFake(SketchDB * db, const timeval & epoch_id, 
    const vector<Flow> & flow_list ) {
    db->PutFlowset_Fake(epoch_id, flow_list);
}

void PutFlowsetWithoutSwitch(SketchDB * db, const timeval & epoch_id, 
    const vector<Flow> & flow_list ) {
    db->PutFlowset_Fake(epoch_id, flow_list);
}


vector<int> TestDelayPutFlowset(SketchDB * db, PutFlowsetFunc PutFlowset) {
    vector<int> result;

    TraceIterator trace_iter(Trace_Path);
    double trace_start_time = (*trace_iter).ts;

    timeval epoch_id = GetEpochId(trace_start_time);
    timeval duration = {duration_ms / 1000, duration_ms % 1000 * 1000};
    timeval end_epoch;
    timeradd(&epoch_id, &duration, &end_epoch);

    unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_map;
    PacketInfo pkt;

    Clock c;
    while(trace_iter.next(&pkt)) {
        //printf("epoch_id: %d\n", GetEpochId(pkt.ts));
        //fflush(stdout);
        if(CmpTimeval(GetEpochId(pkt.ts), epoch_id) != 0) {
            //printf("epoch: %d\n", epoch_id.tv_usec);
            //fflush(stdout);
            vector<Flow> flows_list;
            for(auto flow_pair: flows_map) {
                flows_list.push_back(Flow(flow_pair));
            }
            flows_map.clear();
            //printf("insert size: %lu\n", flows_list.size());
            c.Start();
            PutFlowset(db, epoch_id, flows_list);
            result.push_back(c.Stop());
            epoch_id = GetEpochId(pkt.ts);
            if(CmpTimeval(epoch_id, end_epoch) >= 0) {
                break;
            }
        }
        flows_map[pkt.key].AddPacket(pkt);
    }
    return result;
}

vector<int> TestDelayScan(SketchDB * db) {
    vector<int> result;
    TraceIterator trace_iter(Trace_Path);
    double trace_start_time = (*trace_iter).ts;
    timeval begin_ts = DoubleToTimeval(trace_start_time);
    timeval duration = {duration_ms / 1000, duration_ms % 1000 * 1000};
    timeval end_ts;
    timeradd(&begin_ts, &duration, &end_ts);

    random_device rd;
    minstd_rand gen(rd());
    uniform_int_distribution<> r(0, duration_ms - 100);
    Clock c;
    for(int i = 0; i < 1000; i++) {
        int random_ms = r(gen);
        //printf("ms: %d\n", random_ms);
        timeval random_ts = {random_ms / 1000, random_ms % 1000 * 1000};
        
        timeval scan_start;
        timeval scan_end;
        timeval step = {0, 100 * 1000};
        timeradd(&begin_ts, &random_ts, &scan_start);
        timeradd(&scan_start, &step, &scan_end);
        //printf("%lf, %lf, %lf\n", TimevalToDouble(random_ts), TimevalToDouble(scan_start), TimevalToDouble(scan_end));
        
        vector<Flow> scan_result;
        c.Start();
        db->Scan(scan_start, scan_end, &scan_result);
        result.push_back(c.Stop());
        //printf("%d\n", scan_result.size());
    }
    return result;
}

vector<int> TestDelayGetFlow(SketchDB * db) {
    vector<int> result;
    TraceIterator trace_iter(Trace_Path);
    PacketInfo pkt;

    random_device rd;
    minstd_rand gen(rd());
    uniform_int_distribution<> r(0, 200);
    while(trace_iter.next(&pkt)) {
        if(r(gen) == 0) {
            timeval ts_start = GetEpochId(pkt.ts);
            timeval ts_end;
            timeval ts_range = {1, 0};
            timeradd(&ts_start, &ts_range, &ts_end);
            vector<FlowInfo> getflow_result;
            Clock c;
            c.Start();
            db->GetFlow(pkt.key, ts_start, ts_end, &getflow_result);
            result.push_back(c.Stop());
        }
    }
    return result;
}

void print_result(FILE * file, const vector<int> & result) {
    fprintf(file, "%d", result[0]);
    for(size_t i = 1; i < result.size(); i++) {
        fprintf(file, ",%d", result[i]);
    }
    fprintf(file, "\n");
}

int main(int argc, char ** argv) {
    parse_args(argc, argv);
    
    FILE * eval_output;
    SketchDB * db;
    vector<int> result;

    //PutFlowset
    string PathDelayPutFlowset = Eval_Data_Dir + "delay_PutFlowset.csv";
    eval_output = fopen(PathDelayPutFlowset.c_str(), "w");
    rocksdb::DestroyDB(Db_Path, rocksdb::Options());
    db = new SketchDB(Db_Path);
    result = TestDelayPutFlowset(db, PutFlowset);
    print_result(eval_output, result);
    fclose(eval_output);
    result.clear();

    //Scan
    string PathDelayScan = Eval_Data_Dir + "delay_Scan.csv";
    eval_output = fopen(PathDelayScan.c_str(), "w");
    result = TestDelayScan(db);
    print_result(eval_output, result);
    fclose(eval_output);
    result.clear();

    string PathDelayGetFlow = Eval_Data_Dir + "delay_GetFlow.csv";
    eval_output = fopen(PathDelayGetFlow.c_str(), "w");
    result = TestDelayGetFlow(db);
    print_result(eval_output, result);
    fclose(eval_output);
    result.clear();

    return 0;



    // TraceIterator trace_iter(Trace_Path);
    // double trace_start_time = (*trace_iter).ts;

    // FILE * std_flows = fopen(Output_Path_Std, "w");
    // FILE * eval_output = fopen(Eval_Data_Path, "w");

    // timeval epoch_id = GetEpochId(trace_start_time);
    // timeval duration = {duration_ms / 1000, duration_ms % 1000 * 1000};
    // timeval end_epoch;
    // timeradd(&epoch_id, &duration, &end_epoch);

    // unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_map;
    // PacketInfo pkt;
    // int epoch_cnt = 0;
    // int flow_cnt = 0;
    // int pkt_cnt = 0;
    
    // Clock c;
    // double vm, rss;
    // int insert_time;
    // process_mem_usage(&vm, &rss);
    // while(trace_iter.next(&pkt)) {
    //     if(CmpTimeval(GetEpochId(pkt.ts), epoch_id) != 0) {
    //         vector<Flow> flows_list;
    //         for(auto flow_pair: flows_map) {
    //             flows_list.push_back(Flow(flow_pair));
    //         }
    //         flow_cnt += flows_map.size();
    //         flows_map.clear();
    //         for(auto flow: flows_list) {
    //             flow.Print(std_flows);
    //         }
    //         c.Start();
    //         if(insert_without_index){
    //             db.PutFlowset_Fake(epoch_id, flows_list);
    //         }
    //         else{
    //             db.PutFlowset(epoch_id, flows_list);
    //         }
    //         insert_time = c.Stop();
    //         process_mem_usage(&vm, &rss);
    //         int epoch_time_ms = TimevalToLong(epoch_id) / 1000 % (60 * 1000);
    //         fprintf(eval_output, "%d,%d,%d\n", epoch_time_ms, static_cast<int>(rss), insert_time);
    //         epoch_id = GetEpochId(pkt.ts);
    //         epoch_cnt ++;
    //         if(CmpTimeval(epoch_id, end_epoch) >= 0) {
    //             break;
    //         }
    //     }
    //     flows_map[pkt.key].AddPacket(pkt);
    //     pkt_cnt ++;
    // }
    // fclose(std_flows);

    // vector<Flow> result;
    // epoch_id = GetEpochId(trace_start_time);
    // timeval step = {0, 1000};
    // while (CmpTimeval(epoch_id, end_epoch) < 0) {
    //     db.Get(epoch_id, &result);
    //     timeradd(&epoch_id, &step, &epoch_id);
    // }
    // FILE * db_flows = fopen(Output_Path_Db, "w");
    // for(auto flow: result) {
    //     flow.Print(db_flows);
    // }
    // fclose(db_flows);

    // char cmd[200];
    // sprintf(cmd, "diff -q %s %s", Output_Path_Std, Output_Path_Db);
    // if(system(cmd) == 0) {
    //     printf(ANSI_COLOR_GREEN "[Check OK!]\n" ANSI_COLOR_RESET);
    // }
    // else {
    //     printf(ANSI_COLOR_RED "[Check Failed!]\n" ANSI_COLOR_RESET);
    // }
    // db.Close();
    // rocksdb::DestroyDB(Db_Path, rocksdb::Options());
    // return 0;
}