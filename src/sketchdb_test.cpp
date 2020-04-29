#include <cstdio>
#include <unordered_map>
#include "modules/flow.h"
#include "modules/sketchdb.h"
#include "modules/trace.h"

using namespace sketchstorage;
using namespace std;

const char * Db_Path = "./data/db/test";
const char * Trace_Path = "./data/caida/trace.bin";
const char * Output_Path_Std = "./data/test/true_flows.txt";
const char * Output_Path_Db = "./data/test/db_flows.txt";
const int Duration_ms = 20;
int main(int argc, char ** argv) {
    SketchDB db(Db_Path);

    Trace trace;
    trace.load(Trace_Path);
    
    FILE * std_flows = fopen(Output_Path_Std, "w");

    timeval epoch_id = GetEpochId(trace.start_time());
    timeval duration = {0, Duration_ms * 1000};
    timeval end_epoch;
    timeradd(&epoch_id, &duration, &end_epoch);

    unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_map;
    int epoch_cnt = 0;
    int flow_cnt = 0;
    int pkt_cnt = 0;
    for(auto pkt: trace.packet_list) {
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
            db.Insert(epoch_id, flows_list);
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
    epoch_id = GetEpochId(trace.start_time());
    timeval step = {0, 1000};
    while (CmpTimeval(epoch_id, end_epoch) != 0) {
        db.Read(epoch_id, &result);
        timeradd(&epoch_id, &step, &epoch_id);
    }
    FILE * db_flows = fopen(Output_Path_Db, "w");
    for(auto flow: result) {
        flow.Print(db_flows);
    }
    fclose(db_flows);


    return 0;
}