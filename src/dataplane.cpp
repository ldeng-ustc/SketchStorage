#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>

#include "cxxopts.hpp"

#include "modules/flow.h"
#include "modules/profiling.h"
#include "modules/trace.h"
#include "modules/utility.h"

using namespace std;
using namespace sketchstorage;

cxxopts::ParseResult args;

void parse_args(int argc, char** argv) {
    cxxopts::Options options("dataplane", "Simulate sketch dataplane, aggregate packets to flows periodically, output data to binary file.");
    options.add_options()
        ("e,epoch", "The epoch of flow informations (s).", cxxopts::value<double>()->default_value("0.001"))
        ("o,output", "Output file.", cxxopts::value<string>()->default_value("./data/caida/flow.bin"))
        ("t,duration", "Duration time (ms).", cxxopts::value<uint32_t>()->default_value("1000"))
        ("i,input", "Path to trace binary file.", cxxopts::value<string>()->default_value("./data/caida/trace.bin"))
        ;
    args = options.parse(argc, argv);
}

int main(int argc, char** argv) {
    parse_args(argc, argv);
    FILE *fout;
    fout = fopen(args["output"].as<string>().c_str(), "wb");

    TraceIterator trace_iter(args["input"].as<string>().c_str());
    double trace_start_time = (*trace_iter).ts;
    timeval epoch_id = GetEpochId(trace_start_time, args["epoch"].as<double>());
    timeval duration = {args["duration"].as<uint32_t>() / 1000, args["duration"].as<uint32_t>() % 1000 * 1000};
    timeval end_epoch;
    timeradd(&epoch_id, &duration, &end_epoch);
    unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_map;
    PacketInfo pkt;
    uint64_t total_flow = 0;
    while (trace_iter.next(&pkt)) {
        //printf("epoch_id: %d\n", GetEpochId(pkt.ts));
        //fflush(stdout);
        if (CmpTimeval(GetEpochId(pkt.ts, args["epoch"].as<double>()), epoch_id) != 0) {
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
            total_flow += cnt;
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
    printf("total flow: %lu\n", total_flow);
    // printf("%lu\n", sizeof(FlowBatchItem));
    return 0;
}