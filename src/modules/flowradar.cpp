#include "flowradar.h"
#include "double_hashing.h"

CountingTable::CountingTable(
    uint32_t table_size, 
    uint32_t table_num_hashes,
    uint32_t table_seed
) {
    this->table_size = table_size;
    this->table_num_hashes = table_num_hashes;
    this->counting_table = new FlowradarElement[table_size] ();
    this->seed = table_seed;
}

CountingTable::CountingTable(const CountingTable & b)
:CountingTable(b.table_size, b.table_num_hashes, b.seed) {
    memcpy(this->counting_table, b.counting_table, this->table_size * sizeof(FlowradarElement));
}

CountingTable::~CountingTable() {
    delete this->counting_table;
}

void CountingTable::clear() {
    memset(counting_table, 0, table_size * sizeof(FlowradarElement));
}

void CountingTable::insert(PacketInfo pkt) {
    std::array<uint64_t, 2> hash_values = hash(&(pkt.key), sizeof(pkt.key));
    for(unsigned i=0; i < this->table_num_hashes; i++) {
        uint64_t h = nth_hash(i, hash_values[0], hash_values[1], this->table_size);
        counting_table[h].flow_xor ^= pkt.key;
        counting_table[h].flow_count ++;
        counting_table[h].packet_count ++;
    }
}

void CountingTable::update(PacketInfo pkt) {
    std::array<uint64_t, 2> hash_values = hash(&(pkt.key), sizeof(pkt.key));
    for(unsigned i=0; i < this->table_num_hashes; i++) {
        uint64_t h = nth_hash(i, hash_values[0], hash_values[1], this->table_size);
        counting_table[h].packet_count ++;
    }
}

Flowradar::Flowradar(
    uint32_t filter_size, 
    uint32_t filter_num_hashes, 
    uint32_t table_size, 
    uint32_t table_num_hashes,
    uint32_t filter_seed,
    uint32_t table_seed
):  flow_filter(filter_size, filter_num_hashes, filter_seed),
    counting_table(table_size, table_num_hashes, table_seed) {
}

Flowradar::~Flowradar() {
}

void Flowradar::clear() {
    flow_filter.clear();
    counting_table.clear();
}

void Flowradar::update(PacketInfo pkt) {
    if(! this->flow_filter.contains(&(pkt.key), sizeof(pkt.key))) {
        this->flow_filter.add(&pkt.key, sizeof(pkt.key));
        this->counting_table.insert(pkt);
    }
    else{
        this->counting_table.update(pkt);
    }
}

const CountingTable Flowradar::get_counting_table() {
    return this->counting_table;
}