/* flowradar decoding test.
 * generate random packets to update flowradar, until decoding failed.
 * write result to csv file.
 * 
 * param:
 *  -k: number of hash functions used in flowradar counting table.
 *  -m: flowradar counting table width.
 *  -s: time interval between decodings.
 *  -f: filter size of bloom filter in flowradar.
 *  -h: number of hash functions used in bloom filter in flowradar.
 *  -n: maximum number of intervals, if trace file has more packets, program will ignore the rest.
 *  -i: trace file path.
 *  -o: output file path.
 * 
 */
#include <vector>
#include <cstdio>
#include <functional>
#include <iostream>
#include <bitset>
#include <random>
#include <cassert>

#include <getopt.h>

#include "../modules/packet.h"
#include "../modules/flow.h"
#include "../modules/trace.h"
#include "../modules/controller.h"

using namespace std;

char DEFAULT_PATH[] = "data.csv";
char DEFAULT_TRACE_PATH[] = "./data/caida/trace.bin";

double interval = 0.001;
int max_intervals = 1000;
int table_num_hashes = 3;
int table_size = 1000;
int filter_size = 1<<20;
int filter_num_hashes = 3;
int repeat = 1000;
const char * output_path = DEFAULT_PATH;
const char * trace_path = DEFAULT_TRACE_PATH;

void parse_args (int argc, char **argv)
{
    int c;
    while((c=getopt(argc, argv, "k:m:s:f:h:n:o:i:")) != -1) {
        switch (c)
        {
        case 'k':
            sscanf(optarg, "%d", &table_num_hashes);
            break;
        case 'm':
            sscanf(optarg, "%d", &table_size);
            break;
        case 's':
            sscanf(optarg, "%lf", &interval);
            break;
        case 'f':
            sscanf(optarg, "%d", &filter_size);
            break;
        case 'h':
            sscanf(optarg, "%d", &filter_num_hashes);
            break;
        case 'n':
            sscanf(optarg, "%d", &max_intervals);
            break;
        case 'o':
            output_path = optarg;
            break;
        case 'i':
            trace_path = optarg;
            break;
        default:
            break;
        }
    }
}

int main(int argc, char **argv) {
    parse_args(argc, argv);

    Trace trace;
    trace.load_by_time(trace_path, interval * max_intervals);
    FILE * file = fopen(output_path, "w");
    Flowradar flow_radar(filter_size, filter_num_hashes, table_size, table_num_hashes);
    vector<pair<Flowkey5Tuple, FlowInfo>> out;

    uint64_t current_interval = static_cast<uint64_t>(trace.start_time() / interval);
    int j = 0;
    for(auto pkt: trace.packet_list){
        if(static_cast<uint64_t>(pkt.ts / interval) != current_interval) {
            current_interval ++;
            int failed = Controller::decode(flow_radar.get_counting_table(), out);
            int decoded = out.size();
            assert(failed % table_num_hashes == 0);
            failed = failed / table_num_hashes;

            fprintf(file, "%d,%u\n", failed, decoded + failed);
            flow_radar.clear();
            out.clear();
            j++;
            if(j >= max_intervals) {
                break;
            }
        }
        flow_radar.update(pkt);
        //flow_radar.update(trace.packet_list[0]);
    }

    fprintf(file, "\n");
}