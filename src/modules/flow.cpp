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

FlowIterator::FlowIterator(const char * filename): pos_(0) {
    file_ = fopen(filename, "rb");
    if(file_ == nullptr) {
        printf("failed to open: %s\n", filename);
        end_pos_ = 0;
        return;
    }
    end_pos_ = fread(buf_, sizeof(Flow), FlowIteratorBufferSize, file_);
}

const Flow * FlowIterator::next() {
    if(end_pos_ == 0) {
        return NULL;
    }
    if(pos_ == end_pos_) {
        pos_ = 0;
        end_pos_ = fread(buf_, sizeof(Flow), FlowIteratorBufferSize, file_);
    }
    return &buf_[pos_ ++];
}

Flow FlowIterator::operator * () const {
    if(end_pos_ == 0) {
        throw "TraceIterator: OutOfRange";
    }
    return buf_[pos_];
}

FlowIterator & FlowIterator::operator ++ () {
    if(end_pos_ == 0) {
        throw "FlowIterator: OutOfRange";
    }
    pos_ ++;
    if(pos_ == end_pos_) {
        pos_ = 0;
        end_pos_ = fread(buf_, sizeof(Flow), FlowIteratorBufferSize, file_);
    }
    return *this;
}

FlowBatchIterator::FlowBatchIterator(const char * filename) {
    file_ = fopen(filename, "rb");
    if(file_ == nullptr) {
        printf("failed to open: %s\n", filename);
        return;
    }
}

bool FlowBatchIterator::next(FlowBatch * ret) {
    uint64_t header[2];
    if(fread(header, sizeof(uint64_t), 2, file_) != 2) {
        return false;
    }
    uint64_t epoch_id = header[0];
    uint64_t cnt = header[1];
    ret->epoch_id = epoch_id;
    ret->items.resize(cnt);
    fread(ret->items.data(), sizeof(FlowBatchItem), cnt, file_);
    return true;
}