#include <vector>
#include <string>
#include <cstdio>

#include <pcap/pcap.h>

#include "trace.h"

const int FILENAME_BUF_SIZE = 200;
const char * DEFAULT_OUTPATH = "trace.bin";

int main(int argc, char **argv) {
    if(argc < 3) {
        printf(
            "Read pcap files listed in the input file, "
            "then write packets info to an single binary file."
        );
        printf("Usage:\n%s pcap_dir input_file [output_file]\n", argv[0]);
        return -1;
    }

    FILE *fin = fopen(argv[2], "r");
    std::string dir(argv[1]);
    char filename[FILENAME_BUF_SIZE];
    char errmsg[PCAP_ERRBUF_SIZE];
    const char * output_file = argc < 4 ? DEFAULT_OUTPATH : argv[3];
    bool append = false;
    while(fgets(filename, FILENAME_BUF_SIZE, fin)) {
        std::string full_path = dir + "/" + filename;
        pcap_t * p = pcap_open_offline(full_path.c_str(), errmsg);
        if(p == nullptr) {
            printf("open pcap file %s failed!\n", full_path.c_str());
            printf("errmsg: %s\n", errmsg);
            continue;
        }
        trace_t trace(p);
        trace.save(output_file, append);
        append = true;
        printf("converted: %s\ntrace_size: %d\n", full_path.c_str(), trace.pkt_cnt());
    }
    return 0;
}