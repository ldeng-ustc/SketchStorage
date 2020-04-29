#include "flow.h"
using namespace std;

bool FlowInfo::lt_duration(const FlowInfo & a, const FlowInfo & b) {
    return (a.end_time_ - a.start_time_) < (b.end_time_ - b.start_time_);
}

bool FlowInfo::lt_pkt_cnt(const FlowInfo & a, const FlowInfo & b) {
    return a.pkt_cnt_ < b.pkt_cnt_;
}

bool FlowInfo::lt_size(const FlowInfo & a, const FlowInfo & b) {
    return a.flow_size_ < b.flow_size_;
}

FlowInfo::FlowInfo(): start_time_(0.0), end_time_(0.0), pkt_cnt_(0), flow_size_(0) {
}

void FlowInfo::Merge(const FlowInfo b) {
    start_time_ = min(start_time_, b.start_time_);
    end_time_ = min(end_time_, b.end_time_);
    flow_size_ += b.flow_size_;
    pkt_cnt_ += b.pkt_cnt_;
}

void FlowInfo::AddPacket(const PacketInfo pkt) {
    if(start_time_ == 0) {
        start_time_ = pkt.ts;
    }
    else {
        start_time_ = min(start_time_, pkt.ts);
    }
    end_time_ = max(end_time_, pkt.ts);
    flow_size_ += pkt.size;
    pkt_cnt_ ++;
}

void FlowInfo::Print(FILE * file) const {
    fprintf(file, "start: %f\n", start_time_);
    fprintf(file, "end: %f\n", end_time_);
    fprintf(file, "pkt_cnt: %u\n", pkt_cnt_);
    fprintf(file, "total_size: %u\n", flow_size_);
}

void FlowInfo::Print() const {
    this->Print(stdout);
}

Flow::Flow() {
}

Flow::Flow(pair<Flowkey5Tuple, FlowInfo> key_value) {
    flowkey = key_value.first;
    flowinfo = key_value.second;
}

void Flow::Print(FILE * file) const {
    flowkey.Print(file);
    flowinfo.Print(file);
}

void Flow::Print() const {
    this -> Print(stdout);
}

size_t GetFlowHash(Flowkey5Tuple key) {
    uint64_t hash_value[2];
    MurmurHash3_x64_128(&key, sizeof(key), 0, hash_value);
    return static_cast<size_t>(hash_value[0]);
}

size_t FlowHash::operator()(Flowkey5Tuple const & key) const {
    return GetFlowHash(key);
}