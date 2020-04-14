#include "flow.h"
using namespace std;

bool Flowkey5Tuple::operator < (const Flowkey5Tuple & b) const {
    return memcmp(this, &b, sizeof(Flowkey5Tuple)) < 0;
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