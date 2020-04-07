/* flowradar decoding test.
 * generate random packets to update flowradar, until decoding failed.
 * write result to csv file.
 * 
 * param:
 *  -k: number of hash functions used in flowradar counting table.
 *  -l: minimal test size of flowradar counting table.
 *  -r: maximal test size of flowradar counting table.
 *  -s: counting table size increment after every test.
 *  -f: filter size of bloom filter in flowradar.
 *  -h: number of hash functions used in bloom filter in flowradar.
 *  -n: repeat times for every test.
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

int table_num_hashes = 3;
int table_size_min = 100;
int table_size_max = 2000;
int step = 100;
int filter_size = 1<<20;
int filter_num_hashes = 3;
int repeat = 1000;
char * output_path = DEFAULT_PATH;

int parse_args (int argc, char **argv)
{
    int c;
    while((c=getopt(argc, argv, "k:l:r:s:h:n:o:")) != -1) {
        switch (c)
        {
        case 'k':
            sscanf(optarg, "%d", &table_num_hashes);
            break;
        case 'l':
            sscanf(optarg, "%d", &table_size_min);
            break;
        case 'r':
            sscanf(optarg, "%d", &table_size_max);
            break;
        case 's':
            sscanf(optarg, "%d", &step);
            break;
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
        default:
            break;
        }

    }
}

int main(int argc, char **argv) {
    random_device rd;
    minstd_rand gen(rd());
    parse_args(argc, argv);

    FILE * file = fopen(output_path, "w");
    for(int table_size=table_size_min; table_size<=table_size_max; table_size += step) {
        for(int i=0; i< repeat; i++) {
            int cnt = 0;
            flow_radar_t flow_radar(filter_size, filter_num_hashes, table_size, table_num_hashes);
            vector<pair<flowkey_5_tuple_t, flow_info_t>> out;
            do {
                flow_radar.update(packet_info_t::random(gen));
                cnt ++;
            }while(!flow_radar_controller_t::decode(flow_radar.get_counting_table(), out));
            fprintf(file, i==0 ? "%d" : ",%d", cnt-1);
        }
        fprintf(file, "\n");
    }
}