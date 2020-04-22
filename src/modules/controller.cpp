#include "controller.h"

using namespace std;

Controller::Controller() {
}

Controller::~Controller() {
}

int Controller::decode(
    CountingTable table,
    std::vector<std::pair<Flowkey5Tuple, FlowInfo>> & out
) {
    std::vector<std::pair<Flowkey5Tuple, FlowInfo>> flows_list;
    std::queue<uint32_t> q;
    for(unsigned i=0; i<table.table_size; i++) {
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

        FlowInfo flow_info{0, 0, pkt_cnt, 0};
        out.push_back(std::make_pair(element.flow_xor, flow_info));
        element.flow_count -- ;
        element.flow_xor ^= flow_key;
        element.packet_count = 0;

        auto hash_values = table.hash(&flow_key, sizeof(flow_key));
        for(unsigned i=0; i<table.table_num_hashes; i++) {
            uint64_t h = nth_hash(i, hash_values[0], hash_values[1], table.table_size);
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
    for(unsigned i=0; i<table.table_size; i++) {
        failed_cnt += table.counting_table[i].flow_count;
    }
    return failed_cnt;
}

int Controller::collaborative_decode(
    CountingTable table,
    std::vector<Flowkey5Tuple> known_keys,
    std::vector<std::pair<Flowkey5Tuple, FlowInfo>> & out
){
    while(true) {
        int ret = decode(table, out);

    }
}
