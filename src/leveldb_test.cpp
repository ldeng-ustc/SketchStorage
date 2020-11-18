#include <cassert>
#include <cstdio>
#include <cstring>
#include <climits>
#include <string>
#include <unordered_map>

#include "leveldb/db.h"
#include "leveldb/filter_policy.h"
#include "modules/flow.h"
#include "modules/profiling.h"
#include "modules/trace.h"
#include "modules/utility.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace sketchstorage;

const char* Db_Path = "./data/db/test";
const char* Flow_Path = "./data/caida/flow.bin";
string Eval_Data_Dir = "./data/evaluations/leveldb/";
uint32_t max_batch = UINT32_MAX;

uint64_t global_epoch_st;
uint64_t global_epoch_ed;
vector<Flowkey5Tuple> global_keys;
int key_ratio = 1000;

void parse_args(int argc, char** argv) {
    int c;
    while ((c = getopt(argc, argv, "o:n:")) != -1) {
        switch (c) {
        case 'n':
            sscanf(optarg, "%u", &max_batch);
            break;
        case 'o':
            Eval_Data_Dir = string(optarg);
            break;
        default:
            break;
        }
    }
}

vector<int> testWrite(leveldb::DB *db) {
    vector<int> result;
    FlowBatchIterator batchs(Flow_Path);
    FlowBatch batch;
    char val[1000];
    Clock c;
    int cnt = 0;
    int batch_cnt = 0;
    while (batchs.next(&batch)) {
        if(batch_cnt == 0) {
            cout << "epoch: " << batch.epoch_id << endl;
            global_epoch_st = batch.epoch_id;
        }
        global_epoch_ed = batch.epoch_id;
        batch_cnt ++;
        if(batch_cnt > max_batch) {
            break;
        }
        if(batch_cnt % 100 == 0) {
            cout << "inserting batch " << batch_cnt << "..." << endl;
        }
        // printf("cnt: %lu\n", batch.items.size());
        c.start();
        for(const auto & flow: batch.items) {
            if(rand() % key_ratio == 0) {
                global_keys.push_back(flow.flowkey);
            }
            sprintf(val, "{\"id\": \"%u_%u_%u_%u_%u_%u\", \"t\": \"%lu\", \"size\": %u, \"cnt\": %u}",
                flow.flowkey.src_ip,
                flow.flowkey.dst_ip,
                flow.flowkey.src_port,
                flow.flowkey.dst_port,
                flow.flowkey.proto,
                cnt++,
                batch.epoch_id,
                flow.flow_size,
                flow.pkt_cnt
            );
            auto status = db->Put(leveldb::WriteOptions(), val);
            if(!status.ok()) {
                cout << "status: " << status.ToString() << endl;
            }
#ifdef DEBUG
            char key[1000];
            sprintf(key, "%d_%d_%d_%d_%d_%d",
                flow.flowkey.src_ip,
                flow.flowkey.dst_ip,
                flow.flowkey.src_port,
                flow.flowkey.dst_port,
                flow.flowkey.proto,
                cnt - 1
            );

            string out;
            db->Get(leveldb::ReadOptions(), key, &out);
            printf("val: %s\nout: %s\n", val, out.c_str());
#endif
        }
        result.push_back(c.stop());
    }
    return result;
}

// void Scan(leveldb::DB *db, uint64_t st, uint64_t ed, std::vector<Flow> * result) {
//     uint64_t step = TimevalToLong({0, 1000});
//     char skey[100];
//     vector<leveldb::SKeyReturnVal> vals;
//     for(uint64_t epoch = st; epoch < ed; epoch += step) {
//         sprintf(skey, "%lu", epoch);
//         cout << "read epoch: " << skey << endl;
//         auto status = db->Get(leveldb::ReadOptions(), skey, &vals, 1);
//         if(!status.ok()) {
//             cout << "status: " << status.ToString() << endl;
//             break;
//         }
//         rapidjson::Document docToParse;   
//         for(auto val: vals) {
//             cout << "[" << val.sequence_number << "] " << val.key << ": " << val.value << endl;
//         }
//     } 
// }

// vector<int> testScan(leveldb::DB *db) {
//     return vector<int>();
// }

// void GetFlow(leveldb::DB *db, Flowkey5Tuple flowkey, uint64_t st, uint64_t ed, std::vector<Flow> * result) {
//     char prefix[100];
//     char nprefix[100];
//     sprintf(prefix, "%u_%u_%u_%u_%u",
//         flowkey.src_ip,
//         flowkey.dst_ip,
//         flowkey.src_port,
//         flowkey.dst_port,
//         flowkey.proto
//     );
//     sprintf(nprefix, "%u_%u_%u_%u_%u",
//         flowkey.src_ip,
//         flowkey.dst_ip,
//         flowkey.src_port,
//         flowkey.dst_port,
//         flowkey.proto + 1
//     );
//     auto iter=db->NewIterator(leveldb::ReadOptions());
//     for(iter->Seek(prefix); iter->Valid() && iter->key().compare(nprefix) < 0 ;iter->Next()) {
//         cout << iter->key().ToString() << endl;
//         cout << iter->value().ToString() << endl;
//     }
// }

void print_result(FILE* file, const vector<int>& result) {
    fprintf(file, "%d", result[0]);
    for (size_t i = 1; i < result.size(); i++) {
        fprintf(file, ",%d", result[i]);
    }
    fprintf(file, "\n");
}

int main(int argc, char** argv) {
    parse_args(argc, argv);

    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    options.PrimaryAtt = "id";
    options.secondaryAtt = "t";
    options.filter_policy = leveldb::NewBloomFilterPolicy(10);
    leveldb::Status status = leveldb::DB::Open(options, Db_Path, &db);
    assert(status.ok());

    FILE* eval_output;
    vector<int> result;

    //PutFlowset
    string PathDelayPutFlowset = Eval_Data_Dir + "delay_Write" + ".csv";
    eval_output = fopen(PathDelayPutFlowset.c_str(), "w");
    result = testWrite(db);
    print_result(eval_output, result);
    fclose(eval_output);
    result.clear();

    vector<Flow> res;
    // Scan(db, global_epoch_st, global_epoch_st + TimevalToLong({0, 1000}), &res);
    // GetFlow(db, global_keys[0], 0, 0, &res);
    return 0;
}