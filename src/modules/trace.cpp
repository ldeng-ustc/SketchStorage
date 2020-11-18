#include "trace.h"

Trace::Trace() {};
Trace::Trace(pcap_t * p) {
    const uint8_t *pkt;
    pcap_pkthdr hdr;
    PacketInfo pkt_info;
    while((pkt = pcap_next(p, &hdr)) != NULL) {
        if(decode_packet(hdr, pkt, pkt_info) != 0) {
            continue;
        }
        this->packet_list.push_back(pkt_info);
    }
}

uint32_t Trace::pkt_cnt() {
    return this->packet_list.size();
}

double Trace::start_time() {
    if(this->pkt_cnt() == 0) {
        return 0;
    }
    return this->packet_list[0].ts;
}

double Trace::end_time() {
    if(this->pkt_cnt() == 0) {
        return 0;
    }
    return this->packet_list.rbegin()->ts;
}

int Trace::save(const char * filename, bool append) {
    FILE * file = nullptr;
    if(append) {
        file = fopen(filename, "ab");
    }
    else {
        file = fopen(filename, "wb");
    }

    if(file == nullptr) {
        printf("failed to open: %s\n", filename);
        return -1;
    }

    void * data = this->packet_list.data();
    size_t n = this->packet_list.size();
    size_t cnt = fwrite(data, sizeof(PacketInfo), n, file);
    if(cnt != n){
        printf("failed to write file.\n");
        fclose(file);
        return -cnt;
    }
    return 0;
}

int Trace::load(const char * filename, bool append) {
    FILE * file = fopen(filename, "rb");
    if(!append){
        this->packet_list.clear();
    }
    if(file == nullptr) {
        printf("failed to open: %s\n", filename);
        return -1;
    }
    PacketInfo pkt;
    while(fread(&pkt, sizeof(PacketInfo), 1, file) != 0) {
        this->packet_list.push_back(pkt);
    }
    fclose(file);
    return 0;
}

int Trace::load_by_time(const char *filename, double duration, bool append) {
    FILE * file = fopen(filename, "rb");
    if(!append){
        this->packet_list.clear();
    }
    if(file == nullptr) {
        printf("failed to open: %s\n", filename);
        return -1;
    }
    PacketInfo pkt;
    while(fread(&pkt, sizeof(PacketInfo), 1, file) != 0) {
        this->packet_list.push_back(pkt);
        if(pkt.ts - this->packet_list[0].ts > duration) {
            this->packet_list.pop_back();
            break;
        }
    }
    fclose(file);
    return 0;
}

TraceIterator::TraceIterator(const char * filename): pos_(0) {
    file_ = fopen(filename, "rb");
    if(file_ == nullptr) {
        printf("failed to open: %s\n", filename);
        end_pos_ = 0;
        return;
    }
    end_pos_ = fread(buf_, sizeof(PacketInfo), TraceIteratorBufferSize, file_);
}

bool TraceIterator::next(PacketInfo * pkt) {
    if(end_pos_ == 0) {
        return false;
    }
    *pkt = buf_[pos_ ++];
    if(pos_ == end_pos_) {
        pos_ = 0;
        end_pos_ = fread(buf_, sizeof(PacketInfo), TraceIteratorBufferSize, file_);
    }
    return true;
}

PacketInfo TraceIterator::operator * () const {
    if(end_pos_ == 0) {
        throw "TraceIterator: OutOfRange";
    }
    return buf_[pos_];
}

TraceIterator & TraceIterator::operator ++ () {
    if(end_pos_ == 0) {
        throw "TraceIterator: OutOfRange";
    }
    pos_ ++;
    if(pos_ == end_pos_) {
        pos_ = 0;
        end_pos_ = fread(buf_, sizeof(PacketInfo), TraceIteratorBufferSize, file_);
    }
    return *this;
}
