#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <rocksdb/table.h>
#include <rocksdb/cache.h>
#include <rocksdb/slice.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/db.h>
#include <rocksdb/env.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/statistics.h>

#include "cxxopts.hpp"
#include "modules/profiling.h"

using namespace std;
using namespace rocksdb;

string db_name = "./data/rocksdb_test";
int N = 64 * 1024 * 2;
int B = 256;
uint64_t *buf;

cxxopts::ParseResult args;

void parse_args(int argc, char** argv) {
    cxxopts::Options options("dataplane", "Simulate sketch dataplane, aggregate packets to flows periodically, output data to binary file.");
    options.add_options()
        ("d,database", "Database path.", cxxopts::value<string>()->default_value("./data/rocksdb_test"))
        ("n,samples", "The number of kilo KV pairs, default 64, means 64 * 1024 = 65536 pairs.", cxxopts::value<int>()->default_value("64"))
        ("b,batch", "Batch size.", cxxopts::value<int>()->default_value("256"))
        ;
    args = options.parse(argc, argv);
    db_name = args["database"].as<string>();
    N = args["samples"].as<int>() * 1024 * 2;
    B = args["batch"].as<int>();
    buf = (uint64_t*)malloc(8 * N);
}

DB* open() {
    Clock c;
    DB *db;
    rocksdb::Options options;
    options.create_if_missing = true;
    options.compression = rocksdb::kNoCompression;
    options.max_open_files = 100000;
    // options.write_buffer_size = 64 << 20;
    // options.use_direct_reads = false;
    options.max_background_compactions = 20;
    options.max_bytes_for_level_base = 512 * 1024 * 1024L;

    rocksdb::BlockBasedTableOptions table_options;
    // std::shared_ptr<rocksdb::Cache> block_cache = rocksdb::NewLRUCache(block_cache_size, 1, false, 0.0);
    // table_options.block_cache = block_cache;
    table_options.block_size = 4096;
    table_options.block_restart_interval = 16;

    options.table_factory.reset(NewBlockBasedTableFactory(table_options));

    options.statistics = rocksdb::CreateDBStatistics();

    c.start();
    Status status = DB::Open(options, db_name, &db);
    if(!status.ok()) {
        cout << status.ToString() << endl;
        printf("Open DB failed.\n");
    }
    printf("Open: %lu\n", c.stop());
    return db;
}

int main(int argc, char** argv) {
    parse_args(argc, argv);

    // generate data.
    srand(0);
    for(int i=0; i < N; i++) {
        buf[i] = ((uint64_t)rand()) << 32 + ((uint64_t)rand());
    }

    Clock c;
    c.start();
    DB *db;

    db = open();
    c.start();
    for(int i=0;i<N;i+=2) {
        char *key = (char*)(buf + i);
        char *val = (char*)(buf + i + 1);
        db->Put(WriteOptions(), Slice(key, 64), Slice(val, 64));
    }
    printf("Writes (single pairs): %lu\n", c.stop());
    db->Close();
    DestroyDB(db_name, Options());

    db = open();
    c.start();
    for(int i=0; i+B <= N; i+=B) {
        WriteBatch batch;
        for(int k=0; k<B; k++){
            char *key = (char*)(buf + i + k);
            char *val = (char*)(buf + i + k + 1);
            batch.Put(Slice(key, 64), Slice(val, 64));
        }
        db->Write(WriteOptions(), &batch);
    }
    printf("Writes (batch): %lu\n", c.stop());
    db->Close();
    DestroyDB(db_name, Options());

    db = open();
    c.start();
    for(int i=0; i+B <= N; i+=B) {
        char *key = (char*)(&i);
        char *val = (char*)(buf + i);
        db->Put(WriteOptions(), Slice(key, 4), Slice(val, 200 * 8));
    }
    printf("Writes (merged): %lu\n", c.stop());
    db->Close();
    DestroyDB(db_name, Options());

    return 0;
}