#ifndef __FLOW_H_
#define __FLOW_H_

#include <cstdint>
#include <cstring>

struct __attribute__ ((__packed__)) flowkey_5_tuple_t {
    // 8 (4*2) bytes
    uint32_t src_ip;  // source IP address
    uint32_t dst_ip;
	// 4 (2*2) bytes
    uint16_t src_port;
    uint16_t dst_port;
    // 1 bytes
    uint8_t proto;
    bool operator < (const flowkey_5_tuple_t & b) const {
        return memcmp(this, &b, sizeof(flowkey_5_tuple_t)) < 0;
    }
};

typedef flowkey_5_tuple_t flowkey_t;



struct flowinfo_t {
    double start_time;
    double end_time;
    uint32_t pkt_cnt;
    uint32_t flow_size;

    static bool lt_duration(const flowinfo_t & a, const flowinfo_t & b) {
        return (a.end_time - a.start_time) < (b.end_time - b.start_time);
    }

    static bool lt_pkt_cnt(const flowinfo_t & a, const flowinfo_t & b) {
        return a.pkt_cnt < b.pkt_cnt;
    }

    static bool lt_size(const flowinfo_t & a, const flowinfo_t & b) {
        return a.flow_size < b.flow_size;
    }

};

#endif