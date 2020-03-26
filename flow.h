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

    flowkey_5_tuple_t operator ^= (const flowkey_5_tuple_t & b) {
        this->src_ip ^= b.src_ip;
        this->dst_ip ^= b.dst_ip;
        this->src_port ^= b.src_port;
        this->dst_port ^= b.dst_port;
        this->proto ^= b.proto;
    }
};

struct flow_info_t {
    double start_time;
    double end_time;
    uint32_t pkt_cnt;
    uint32_t flow_size;

    static bool lt_duration(const flow_info_t & a, const flow_info_t & b) {
        return (a.end_time - a.start_time) < (b.end_time - b.start_time);
    }

    static bool lt_pkt_cnt(const flow_info_t & a, const flow_info_t & b) {
        return a.pkt_cnt < b.pkt_cnt;
    }

    static bool lt_size(const flow_info_t & a, const flow_info_t & b) {
        return a.flow_size < b.flow_size;
    }

};

#endif