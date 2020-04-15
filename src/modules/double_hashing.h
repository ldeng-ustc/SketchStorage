#ifndef __DOUBLE_HASHING_H_
#define __DOUBLE_HASHING_H_

#include <cstdint>

uint64_t nth_hash(uint8_t n, uint64_t hash1, uint64_t hash2, uint64_t size);

#endif