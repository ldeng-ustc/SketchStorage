#include "double_hashing.h"

uint64_t nth_hash(uint8_t n, uint64_t hash1, uint64_t hash2, uint64_t size) {
    return (hash1 + n * hash2 + n * n) % size;
}