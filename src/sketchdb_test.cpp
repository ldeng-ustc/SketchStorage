#include <cstdio>
#include <unordered_map>
#include "modules/flow.h"
#include "modules/sketchdb.h"
#include "modules/trace.h"

using namespace sketchstorage;
using namespace std;

int main(int argc, char ** argv) {
    SketchDB db("/data/db/test");

    Trace trace;
    trace.load("/data/caida/trace.bin");
    
    uint64_t epoch_id = GetEpochId(trace.start_time);
    unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_map;
    for(auto pkt: trace.packet_list) {
        if(GetEpochId(pkt.ts) != epoch_id) {
            vector<Flow> flows_list;
            for(auto flow_pair: flows_map) {
                flows_list.push_back(Flow(flow_pair));
            }
            flows_map.clear();
            epoch_id = GetEpochId(pkt.ts);
        }
        flows_map[pkt.key].AddPacket(pkt);
    }

}