#include "flow.h"
using namespace std;

bool Flowkey5Tuple::operator < (const Flowkey5Tuple & b) const {
    return memcmp(this, &b, sizeof(Flowkey5Tuple)) < 0;
}

bool Flowkey5Tuple::operator <= (const Flowkey5Tuple & b) const {
    return memcmp(this, &b, sizeof(Flowkey5Tuple)) <= 0;
}

bool Flowkey5Tuple::operator > (const Flowkey5Tuple & b) const {
    return memcmp(this, &b, sizeof(Flowkey5Tuple)) > 0;
}

bool Flowkey5Tuple::operator >= (const Flowkey5Tuple & b) const {
    return memcmp(this, &b, sizeof(Flowkey5Tuple)) >= 0;
}

bool Flowkey5Tuple::operator == (const Flowkey5Tuple & b) const {
    return memcmp(this, &b, sizeof(Flowkey5Tuple)) == 0;
}

bool Flowkey5Tuple::operator != (const Flowkey5Tuple & b) const {
    return memcmp(this, &b, sizeof(Flowkey5Tuple)) != 0;
}

Flowkey5Tuple Flowkey5Tuple::operator ^= (const Flowkey5Tuple & b) {
    this->src_ip ^= b.src_ip;
    this->dst_ip ^= b.dst_ip;
    this->src_port ^= b.src_port;
    this->dst_port ^= b.dst_port;
    this->proto ^= b.proto;
    return *this;
}

Flowkey5Tuple Flowkey5Tuple::random(minstd_rand & gen) {
    std::uniform_int_distribution<uint32_t> r;
    Flowkey5Tuple key;
    key.src_ip = r(gen);
    key.dst_ip = r(gen);
    key.src_port = static_cast<uint16_t>(r(gen));
    key.dst_port = static_cast<uint16_t>(r(gen));
    key.proto = static_cast<uint8_t>(r(gen));
    return key;
}

bool FlowInfo::lt_duration(const FlowInfo & a, const FlowInfo & b) {
    return (a.end_time_ - a.start_time_) < (b.end_time_ - b.start_time_);
}

bool FlowInfo::lt_pkt_cnt(const FlowInfo & a, const FlowInfo & b) {
    return a.pkt_cnt_ < b.pkt_cnt_;
}

bool FlowInfo::lt_size(const FlowInfo & a, const FlowInfo & b) {
    return a.flow_size_ < b.flow_size_;
}

void FlowInfo::Merge(const FlowInfo b) {
    start_time_ = min(start_time_, b.start_time_);
    end_time_ = min(end_time_, b.end_time_);
    flow_size_ += b.flow_size_;
    pkt_cnt_ += b.pkt_cnt_;
}

Flow::Flow() {
}

Flow::Flow(pair<Flowkey5Tuple, FlowInfo> key_value) {
    flowkey = key_value.first;
    flowinfo = key_value.second;
}


size_t GetFlowHash(Flowkey5Tuple key) {
    uint64_t hash_value[2];
    MurmurHash3_x64_128(&key, sizeof(key), 0, hash_value);
    return static_cast<size_t>(hash_value[0]);
}

size_t FlowHash::operator()(Flowkey5Tuple const & key) const {
    return GetFlowHash(key);
}