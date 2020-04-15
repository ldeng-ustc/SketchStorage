#ifndef __TRACE_H_
#define __TRACE_H_

#include <vector>
#include <cstdint>

#include <pcap/pcap.h>

#include "packet.h"

class Trace {
public:
    std::vector<PacketInfo> packet_list;
    Trace();
    Trace(pcap_t * p);
    uint32_t pkt_cnt();
    double start_time();
    double end_time();
    int save(const char * filename, bool append=false);
    int load(const char * filename, bool append=false);
    int load_by_time(const char *filename, double duration, bool append=false);
};

#endif