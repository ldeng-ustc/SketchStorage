#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

#include "modules/flow.h"
#include "modules/profiling.h"
#include "modules/trace.h"
#include "modules/utility.h"

using namespace std;
using namespace sketchstorage;

const char* Trace_Path = "./data/caida/trace.bin";
const char* Output_Path = "./data/caida/flow.bin";
int duration_ms = 60000;
int scan_ms = 100;
bool insert_without_index = false;

void parse_args(int argc, char** argv) {
    int c;
    while ((c = getopt(argc, argv, "d:o:t:")) != -1) {
        switch (c) {
        case 'd':
            sscanf(optarg, "%d", &duration_ms);
            break;
        case 'o':
            Output_Path = optarg;
            break;
        case 't':
            sscanf(optarg, "%d", &scan_ms);
        default:
            break;
        }
    }
}

int main(int argc, char** argv) {
    parse_args(argc, argv);
    FILE *fout;
    fout = fopen(Output_Path, "wb");

    TraceIterator trace_iter(Trace_Path);
    double trace_start_time = (*trace_iter).ts;
    timeval epoch_id = GetEpochId(trace_start_time);
    timeval duration = {duration_ms / 1000, duration_ms % 1000 * 1000};
    timeval end_epoch;
    timeradd(&epoch_id, &duration, &end_epoch);
    unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_map;
    PacketInfo pkt;
    while (trace_iter.next(&pkt)) {
        //printf("epoch_id: %d\n", GetEpochId(pkt.ts));
        //fflush(stdout);
        if (CmpTimeval(GetEpochId(pkt.ts), epoch_id) != 0) {
            //printf("epoch: %d\n", epoch_id.tv_usec);
            //fflush(stdout);
            vector<Flow> flows_list;
            for (auto flow_pair : flows_map) {
                flows_list.push_back(Flow(flow_pair));
            }
            flows_map.clear();
            uint64_t epoch_id_ull = TimevalToLong(epoch_id);
            uint64_t cnt = flows_list.size();
            fwrite(&epoch_id_ull, sizeof(uint64_t), 1, fout);
            fwrite(&cnt, sizeof(uint64_t), 1, fout);
            for(auto flow: flows_list) {
                FlowBatchItem item = {flow.flowkey, flow.flowinfo.flow_size_, flow.flowinfo.pkt_cnt_};
                fwrite(&item, sizeof(FlowBatchItem), 1, fout);
            }

            epoch_id = GetEpochId(pkt.ts);
            if (CmpTimeval(epoch_id, end_epoch) >= 0) {
                break;
            }
        }
        flows_map[pkt.key].AddPacket(pkt);
    }

    return 0;
}