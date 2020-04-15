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

public:
    bloom_filter_t(uint32_t size, uint32_t num_hashes, uint32_t seed=0);
    ~bloom_filter_t();
    void add(const void *data, std::size_t len);
    bool contains(const void *data, std::size_t len);
    void clear();
};

#endif