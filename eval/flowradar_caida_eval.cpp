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
 *  -n: repeat times for every test.
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

#include <getopt.h>

#include "../packet.h"
#include "../flow.h"
#include "../trace.h"
#include "../flowradar.h"

using namespace std;

char DEFAULT_PATH[] = "data.csv";
char DEFAULT_TRACE_PATH[] = "./data/caida/trace.bin";

double interval = 0.001;
int table_num_hashes = 3;
int table_size = 600;
int filter_size = 1<<20;
int filter_num_hashes = 3;
int repeat = 1000;
const char * output_path = DEFAULT_PATH;
const char * trace_path = DEFAULT_TRACE_PATH;

int parse_args (int argc, char **argv)
{
    int c;
    while((c=getopt(argc, argv, "k:l:r:s:h:n:o:")) != -1) {
        switch (c)
        {
        case 'k':
            sscanf(optarg, "%d", &table_num_hashes);
            break;
        case 'm':
            sscanf(optarg, "%d", &table_size);
            break;
        case 's':

        case 'f':
            sscanf(optarg, "%d", &filter_size);
            break;
        case 'h':
            sscanf(optarg, "%d", &filter_num_hashes);
            break;
        case 'n':
            sscanf(optarg, "%d", &repeat);
            break;
        case 'o':
            output_path = optarg;
            break;
        case 'i':
            trace_path = optarg;
        default:
            break;
        }

    }
}

typedef flow_radar_controller_t controller_t;

int main(int argc, char **argv) {
    parse_args(argc, argv);

    trace_t trace;
    trace.load(trace_path);
    FILE * file = fopen(output_path, "w");
    for(int i=0; i< repeat; i++) {
        int cnt = 0;
        flow_radar_t flow_radar(filter_size, filter_num_hashes, table_size, table_num_hashes);
        vector<pair<flowkey_5_tuple_t, flow_info_t>> out;

        int current_interval = (int)trace.start_time();
        for(auto pkt: trace.packet_list){
            if((int)pkt.ts != current_interval) {
                int failed = controller_t::decode(flow_radar.get_counting_table(), out);
                fprintf(file, i==0 ? "%d" : ",%d", failed/table_num_hashes);
            }
        }
    }
    fprintf(file, "\n");
}