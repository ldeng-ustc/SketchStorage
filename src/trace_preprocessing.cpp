#include <vector>
#include <string>
#include <cstdio>

#include <pcap/pcap.h>

#include "./modules/trace.h"

const int FILENAME_BUF_SIZE = 200;
const char * DEFAULT_PCAP_DIR = "./data/caida";
const char * DEFAULT_INPUT_PATH = "./data/caida/pcap_list.txt";
const char * DEFAULT_OUTPUT_PATH = "./data/caida/trace.bin";

int main(int argc, char **argv) {
    if(argc < 3) {
        printf(
            "Read pcap files listed in the input file, "
            "then write packets info to an single binary file."
        );
        printf("Usage:\n%s pcap_dir input_file [output_file]\n", argv[0]);
    }

    const char * pcap_dir = argc < 2 ? DEFAULT_PCAP_DIR : argv[1];
    const char * input_file = argc < 3 ? DEFAULT_INPUT_PATH : argv[2]; 
    const char * output_file = argc < 4 ? DEFAULT_OUTPUT_PATH : argv[3];


    FILE *fin = fopen(input_file, "r");
    std::string dir(pcap_dir);
    char filename[FILENAME_BUF_SIZE];
    char errmsg[PCAP_ERRBUF_SIZE];
    bool append = false;
    while(fgets(filename, FILENAME_BUF_SIZE, fin)) {
        std::string full_path = dir + "/" + filename;
        pcap_t * p = pcap_open_offline(full_path.c_str(), errmsg);
        if(p == nullptr) {
            printf("open pcap file %s failed!\n", full_path.c_str());
            printf("errmsg: %s\n", errmsg);
            continue;
        }
        Trace trace(p);
        trace.save(output_file, append);
        append = true;
        printf("converted: %s\ntrace_size: %d\n", full_path.c_str(), trace.pkt_cnt());
    }
    return 0;
}