#ifndef __FLOWRADAR_H_
#define __FLOWRADAR_H_

#include <cstdint>
#include <array>
#include <vector>
#include <queue>
#include <utility>

#include "packet.h"
#include "flow.h"
#include "bloom_filter.h"
#include "MurmurHash3.h"

#pragma pack (1)
struct FlowradarElement {
    Flowkey5Tuple flow_xor;
    uint8_t flow_count;
    uint16_t packet_count;
};

class CountingTable {
public:
    uint32_t table_size;
    uint32_t table_num_hashes;
    uint32_t seed;
    FlowradarElement * counting_table;

    inline std::array<uint64_t, 2> hash(const void *data, std::size_t len) {
        std::array<uint64_t, 2> hash_value;
        MurmurHash3_x64_128(data, len, this->seed, hash_value.data());
        return hash_value;
    }

    CountingTable(uint32_t table_size, uint32_t table_num_hashes, uint32_t table_seed = 1);
    CountingTable(const CountingTable & b);
    ~CountingTable();
    void clear();
    void insert(PacketInfo pkt);
    void update(PacketInfo pkt);
};


class Flowradar {
private:
    bloom_filter_t flow_filter;
    CountingTable counting_table;
public:
    Flowradar(
        uint32_t filter_size, 
        uint32_t filter_num_hashes, 
        uint32_t table_size, 
        uint32_t table_num_hashes,
        uint32_t filter_seed = 0,
        uint32_t table_seed = 1
    );
    ~Flowradar();
    void clear();
    void update(PacketInfo pkt);
    const CountingTable get_counting_table();
};

#endif