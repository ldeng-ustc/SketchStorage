#ifndef __TRACE_H_
#define __TRACE_H_

#include <vector>
#include <cstdint>

#include <pcap/pcap.h>

#include "packet.h"

class trace_t {
public:
    std::vector<packet_info_t> packet_list;
    trace_t() {};
    trace_t(pcap_t * p) {
        const uint8_t *pkt;
        pcap_pkthdr hdr;
        packet_info_t pkt_info;
        while((pkt = pcap_next(p, &hdr)) != NULL) {
            if(decode_packet(hdr, pkt, pkt_info) != 0) {
                continue;
            }
            this->packet_list.push_back(pkt_info);
        }
    }

    uint32_t pkt_cnt() {
        return this->packet_list.size();
    }

    double start_time() {
        if(this->pkt_cnt() == 0) {
            return 0;
        }
        return this->packet_list[0].ts;
    }

    double end_time() {
        if(this->pkt_cnt() == 0) {
            return 0;
        }
        return this->packet_list.rbegin()->ts;
    }

    int save(const char * filename, bool append=false) {
        FILE * file = nullptr;
        if(append) {
            file = fopen(filename, "ab");
        }
        else {
            file = fopen(filename, "wb");
        }

        if(file == nullptr) {
            printf("failed to open: %s\n", filename);
            return -1;
        }

        void * data = this->packet_list.data();
        size_t n = this->packet_list.size();
        size_t cnt = fwrite(data, sizeof(packet_info_t), n, file);
        if(cnt != n){
            printf("failed to write file.\n");
            fclose(file);
            return -cnt;
        }
        return 0;
    }

    int load(const char * filename, bool append=false) {
        FILE * file = fopen(filename, "rb");
        if(!append){
            this->packet_list.clear();
        }
        if(file == nullptr) {
            printf("failed to open: %s\n", filename);
            return -1;
        }
        packet_info_t pkt;
        while(fread(&pkt, sizeof(packet_info_t), 1, file) != 0) {
            this->packet_list.push_back(pkt);
        }
        fclose(file);
        return 0;
    }

    int load_by_time(const char *filename, double duration, bool append=false) {
        FILE * file = fopen(filename, "rb");
        if(!append){
            this->packet_list.clear();
        }
        if(file == nullptr) {
            printf("failed to open: %s\n", filename);
            return -1;
        }
        packet_info_t pkt;
        while(fread(&pkt, sizeof(packet_info_t), 1, file) != 0) {
            this->packet_list.push_back(pkt);
            if(pkt.ts - this->packet_list[0].ts > duration) {
                this->packet_list.pop_back();
                break;
            }
        }
        fclose(file);
        return 0;
    }
};


#endif