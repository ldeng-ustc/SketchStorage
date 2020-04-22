#ifndef __FLOW_H_
#define __FLOW_H_

#include <random>
#include <cstdint>
#include <cstring>

#include "MurmurHash3.h"

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
    bool operator <= (const Flowkey5Tuple & b) const;
    bool operator > (const Flowkey5Tuple & b) const;
    bool operator >= (const Flowkey5Tuple & b) const;
    bool operator == (const Flowkey5Tuple & b) const;
    bool operator != (const Flowkey5Tuple & b) const;
    Flowkey5Tuple operator ^= (const Flowkey5Tuple & b);
    static Flowkey5Tuple random(std::minstd_rand & gen);
};

class FlowInfo {
public:
    double start_time_;
    double end_time_;
    uint32_t pkt_cnt_;
    uint32_t flow_size_;

    static bool lt_duration(const FlowInfo & a, const FlowInfo & b);
    static bool lt_pkt_cnt(const FlowInfo & a, const FlowInfo & b);
    static bool lt_size(const FlowInfo & a, const FlowInfo & b);

    void Merge(const FlowInfo b);
};

struct Flow{
    Flowkey5Tuple flowkey;
    FlowInfo flowinfo;

    Flow();
    Flow(std::pair<Flowkey5Tuple, FlowInfo> key_value);
};

size_t GetFlowHash(Flowkey5Tuple key);

struct FlowHash {
    std::size_t operator()(Flowkey5Tuple const & key) const;
};


#endif