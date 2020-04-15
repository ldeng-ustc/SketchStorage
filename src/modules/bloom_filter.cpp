#include "bloom_filter.h"
#include "double_hashing.h"

bloom_filter_t::bloom_filter_t(uint32_t size, uint32_t num_hashes, uint32_t seed) {
    this->size = size;
    this->num_hashes = num_hashes;
    this->seed = seed;
    bits = new uint8_t[size / 8] ();
}

bloom_filter_t::~bloom_filter_t() {
    delete this->bits;
    this->bits = nullptr;
}

void bloom_filter_t::add(const void *data, std::size_t len) {
    auto hash_values = hash(data, len);
    for(unsigned i=0; i<num_hashes; i++) {
        uint64_t h = nth_hash(i, hash_values[0], hash_values[1], this->size);
        this->set(h);
    }
}

bool bloom_filter_t::contains(const void *data, std::size_t len) {
    auto hash_values = hash(data, len);
    for(unsigned i=0; i<num_hashes; i++) {
        uint64_t h = nth_hash(i, hash_values[0], hash_values[1], this->size);
        if(! this->test(h)) {
            return false;
        }
    }
    return true;
}

void bloom_filter_t::clear() {
    memset(this->bits, 0, this->size / 8);
}
