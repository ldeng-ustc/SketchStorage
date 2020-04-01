#ifndef _FLOW_RADAR_H__
#define _FLOW_RADAR_H__

#include <cstdint>
#include <array>
#include <vector>
#include <queue>
#include <utility>

#include "packet.h"
#include "flow.h"
#include "bloom_filter.h"
#include "MurmurHash3.h"

flow_info_t flow;

struct __attribute__ ((__packed__)) flow_radar_element_t {
    flowkey_5_tuple_t flow_xor;
    uint8_t flow_count;
    uint16_t packet_count;
};

class flow_radar_counting_table_t {
public:
    uint32_t table_size;
    uint32_t table_num_hashes;
    uint32_t seed;
    flow_radar_element_t * counting_table;

    inline std::array<uint64_t, 2> hash(const void *data, std::size_t len) {
        std::array<uint64_t, 2> hash_value;
        MurmurHash3_x64_128(data, len, this->seed, hash_value.data());
        return hash_value;
    }

    inline uint64_t nth_hash(uint8_t n, uint64_t hash1, uint64_t hash2, uint64_t size) {
        return (hash1 + n * hash2) % size;
    }

    flow_radar_counting_table_t(
        uint32_t table_size, 
        uint32_t table_num_hashes,
        uint32_t table_seed = 1
    );
    flow_radar_counting_table_t(const flow_radar_counting_table_t & b);
    ~flow_radar_counting_table_t();
    void clear();
    void insert(packet_info_t pkt);
    void update(packet_info_t pkt);
};

flow_radar_counting_table_t::flow_radar_counting_table_t(
    uint32_t table_size, 
    uint32_t table_num_hashes,
    uint32_t table_seed
) {
    this->table_size = table_size;
    this->table_num_hashes = table_num_hashes;
    this->counting_table = new flow_radar_element_t[table_size] ();
    this->seed = table_seed;
}

flow_radar_counting_table_t::flow_radar_counting_table_t(const flow_radar_counting_table_t & b)
:flow_radar_counting_table_t(b.table_size, b.table_num_hashes, b.seed) {
    memcpy(this->counting_table, b.counting_table, this->table_size * sizeof(flow_radar_element_t));
}

flow_radar_counting_table_t::~flow_radar_counting_table_t() {
    delete this->counting_table;
}

void flow_radar_counting_table_t::clear() {
    memset(counting_table, 0, table_size * sizeof(flow_radar_element_t));
}

void flow_radar_counting_table_t::insert(packet_info_t pkt) {
    std::array<uint64_t, 2> hash_values = hash(&(pkt.key), sizeof(pkt.key));
    for(int i=0; i < this->table_num_hashes; i++) {
        uint64_t h = nth_hash(i, hash_values[0], hash_values[1], this->table_size);
        counting_table[h].flow_xor ^= pkt.key;
        counting_table[h].flow_count ++;
        counting_table[h].packet_count ++;
    }
}

void flow_radar_counting_table_t::update(packet_info_t pkt) {
    std::array<uint64_t, 2> hash_values = hash(&(pkt.key), sizeof(pkt.key));
    for(int i=0; i < this->table_num_hashes; i++) {
        uint64_t h = nth_hash(i, hash_values[0], hash_values[1], this->table_size);
        counting_table[h].packet_count ++;
    }
}


class flow_radar_t {
private:
    bloom_filter_t flow_filter;
    flow_radar_counting_table_t counting_table;
public:
    flow_radar_t(
        uint32_t filter_size, 
        uint32_t filter_num_hashes, 
        uint32_t table_size, 
        uint32_t table_num_hashes,
        uint32_t filter_seed = 0,
        uint32_t table_seed = 1
    );
    ~flow_radar_t();
    void clear();
    void update(packet_info_t pkt);
    const flow_radar_counting_table_t get_counting_table();
};

flow_radar_t::flow_radar_t(
    uint32_t filter_size, 
    uint32_t filter_num_hashes, 
    uint32_t table_size, 
    uint32_t table_num_hashes,
    uint32_t filter_seed,
    uint32_t table_seed
):  flow_filter(filter_size, filter_num_hashes, filter_seed),
    counting_table(table_size, table_num_hashes, table_seed) {
}

flow_radar_t::~flow_radar_t() {
}

void flow_radar_t::clear() {
    flow_filter.clear();
    counting_table.clear();
}

void flow_radar_t::update(packet_info_t pkt) {
    if(! this->flow_filter.contains(&(pkt.key), sizeof(pkt.key))) {
        this->counting_table.insert(pkt);
    }
    else{
        this->counting_table.update(pkt);
    }
}

const flow_radar_counting_table_t flow_radar_t::get_counting_table() {
    return this->counting_table;
}

class flow_radar_controller_t
{
private:
    /* data */
public:
    flow_radar_controller_t(/* args */);
    ~flow_radar_controller_t();

    static int decode(
        flow_radar_counting_table_t table, 
        std::vector<std::pair<flowkey_5_tuple_t, flow_info_t>> & out
    );
};

flow_radar_controller_t::flow_radar_controller_t(/* args */) {
}

flow_radar_controller_t::~flow_radar_controller_t() {
}

int flow_radar_controller_t::decode(
    flow_radar_counting_table_t table,
    std::vector<std::pair<flowkey_5_tuple_t, flow_info_t>> & out
) {
    std::vector<std::pair<flowkey_5_tuple_t, flow_info_t>> flows_list;
    std::queue<uint32_t> q;
    for(int i=0; i<table.table_size; i++) {
        if(table.counting_table[i].flow_count == 1) {
            q.push(i);
        }
    }
    while(!q.empty()) {
        uint32_t index = q.front();
        q.pop();

        auto & element = table.counting_table[index];
        auto flow_key = element.flow_xor;
        auto pkt_cnt = element.packet_count;

        if(element.flow_count == 0) {   // already in flow_list.
            continue;
        }

        flow_info_t flow_info{0, 0, pkt_cnt, 0};
        out.push_back(std::make_pair(element.flow_xor, flow_info));
        element.flow_count -- ;
        element.flow_xor ^= flow_key;
        element.packet_count = 0;

        auto hash_values = table.hash(&flow_key, sizeof(flow_key));
        for(int i=0; i<table.table_num_hashes; i++) {
            uint64_t h = table.nth_hash(i, hash_values[0], hash_values[1], table.table_size);
            if(h != index) {
                table.counting_table[h].flow_xor ^= flow_key;
                table.counting_table[h].flow_count --;
                table.counting_table[h].packet_count -= pkt_cnt;
                if(table.counting_table[h].flow_count == 1) {
                    q.push(h);
                }
            }
        }
    }

    uint32_t failed_cnt = 0;
    for(int i=0; i<table.table_size; i++) {
        failed_cnt += table.counting_table[i].flow_count;
    }
    return failed_cnt;
}

#endif