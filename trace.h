#ifndef __TRACE_H_
#define __TRACE_H_

#include <vector>
#include <cstdint>

#include <pcap/pcap.h>

#include "packet.h"

class trace_t {
public:
    uint32_t pkt_cnt;
    double start_time;
    double end_time;
    std::vector<packet_info_t> packet_list;

    trace_t(): pkt_cnt(0) {}
    trace_t(pcap_t * p): pkt_cnt(0) {
        const uint8_t *pkt;
        pcap_pkthdr hdr;
        packet_info_t pkt_info;
        while((pkt = pcap_next(p, &hdr)) != NULL) {
            if(decode_packet(hdr, pkt, pkt_info) != 0) {
                continue;
            }
            const flowkey_5_tuple_t & key = pkt_info.key;
            const double & pkt_ts = pkt_info.ts; 

            if(this->pkt_cnt == 0) {
                this->start_time = pkt_ts;
            }
            this->pkt_cnt ++;
            this->end_time = pkt_ts;
            this->packet_list.push_back(pkt_info);
        }
        
    }
};


#endif