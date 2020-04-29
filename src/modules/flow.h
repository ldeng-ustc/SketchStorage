#ifndef __FLOW_H_
#define __FLOW_H_

#include <random>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <arpa/inet.h>

#include "flowkey.h"
#include "packet.h"
#include "MurmurHash3.h"



class FlowInfo {
public:
    double start_time_;
    double end_time_;
    uint32_t pkt_cnt_;
    uint32_t flow_size_;

    static bool lt_duration(const FlowInfo & a, const FlowInfo & b);
    static bool lt_pkt_cnt(const FlowInfo & a, const FlowInfo & b);
    static bool lt_size(const FlowInfo & a, const FlowInfo & b);

    FlowInfo();
    void Merge(const FlowInfo b);
    void AddPacket(const PacketInfo pkt);
    void Print(FILE * file) const;
    void Print() const;
};

#pragma pack(8)
struct Flow{
    Flowkey5Tuple flowkey;
    FlowInfo flowinfo;

    Flow();
    Flow(std::pair<Flowkey5Tuple, FlowInfo> key_value);
    void Print(FILE * file) const;
    void Print() const;
};

size_t GetFlowHash(Flowkey5Tuple key);

struct FlowHash {
    std::size_t operator()(Flowkey5Tuple const & key) const;
};


#endif