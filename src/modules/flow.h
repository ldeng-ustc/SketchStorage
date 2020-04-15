#ifndef __FLOW_H_
#define __FLOW_H_

#include <random>
#include <cstdint>
#include <cstring>

#pragma pack (1)
struct Flowkey5Tuple {
    // 8 (4*2) bytes
    uint32_t src_ip;  // source IP address
    uint32_t dst_ip;
	// 4 (2*2) bytes
    uint16_t src_port;
    uint16_t dst_port;
    // 1 bytes
    uint8_t proto;
    
    bool operator < (const Flowkey5Tuple & b) const;
    Flowkey5Tuple operator ^= (const Flowkey5Tuple & b);
    static Flowkey5Tuple random(std::minstd_rand & gen);
};

struct FlowInfo {
    double start_time;
    double end_time;
    uint32_t pkt_cnt;
    uint32_t flow_size;

    static bool lt_duration(const FlowInfo & a, const FlowInfo & b);
    static bool lt_pkt_cnt(const FlowInfo & a, const FlowInfo & b);
    static bool lt_size(const FlowInfo & a, const FlowInfo & b);
};

#endif