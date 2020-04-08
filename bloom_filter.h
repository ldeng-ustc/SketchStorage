#ifndef __BLOOM_FILTER_H_
#define __BLOOM_FILTER_H_

#include <cstdint>
#include <cstring>
#include <array>

#include "MurmurHash3.h"


class bloom_filter_t {
private:
    uint8_t * bits;
    uint32_t num_hashes;
    uint32_t size;
    uint32_t seed;

    inline void set(uint32_t i) {
        bits[i/256] |= (1<<(i % 8));
    }

    inline bool test(uint32_t i) {
        return (bits[i/256] & (1<<(i % 8))) != 0;
    }

    inline std::array<uint64_t, 2> hash(const void *data, std::size_t len) {
        std::array<uint64_t, 2> hash_value;
        MurmurHash3_x64_128(data, len, this->seed, hash_value.data());
        return hash_value;
    }

    inline uint64_t nth_hash(uint8_t n, uint64_t hash1, uint64_t hash2, uint64_t size) {
        return (hash1 + n * hash2 + n * n) % size;
    }

public:
    bloom_filter_t(uint32_t size, uint32_t num_hashes, uint32_t seed=0);
    ~bloom_filter_t();
    void add(const void *data, std::size_t len);
    bool contains(const void *data, std::size_t len);
    void clear();
};

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

#endif