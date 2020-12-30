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
int L = 1;
uint64_t *buf;

cxxopts::ParseResult args;

void parse_args(int argc, char** argv) {
    cxxopts::Options options("dataplane", "Simulate sketch dataplane, aggregate packets to flows periodically, output data to binary file.");
    options.add_options()
        ("d,database", "Database path.", cxxopts::value<string>()->default_value("./data/rocksdb_test"))
        ("n,samples", "The number of kilo KV pairs, default 64, means 64 * 1024 = 65536 pairs.", cxxopts::value<int>()->default_value("64"))
        ("b,batch", "Batch size.", cxxopts::value<int>()->default_value("256"))
        ("l,loop", "Loop times.", cxxopts::value<int>()->default_value("1"))
        ;
    args = options.parse(argc, argv);
    db_name = args["database"].as<string>();
    N = args["samples"].as<int>() * 1024 * 2;
    B = args["batch"].as<int>();
    L = args["loop"].as<int>();
    buf = (uint64_t*)malloc(8 * N);
}

DB* open() {
    Clock c;
    DB *db;
    rocksdb::Options options;
    options.create_if_missing = true;
    options.compression = rocksdb::kNoCompression;
    options.max_open_files = 100000;
    options.write_buffer_size = 64 << 20;
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

    c.Start();
    Status status = DB::Open(options, db_name, &db);
    if(!status.ok()) {
        cout << status.ToString() << endl;
        printf("Open DB failed.\n");
    }
    printf("Open: %f\n", c.Stop());
    return db;
}

void generate_data(int seed) {
    Clock c;
    srand(seed);
    for(int i=0; i < N; i++) {
        buf[i] = ((uint64_t)rand()) << 32 + ((uint64_t)rand());
    }
    printf("Gen data: %f\n", c.Stop());
}

void test_puts() {
    Clock c;
    DB *db;
    db = open();
    c.Start();
    for(int l=0; l<L; l++) {
        c.Pause();
        generate_data(l);
        c.Restart();
        for(int i=0; i<N; i+=2) {
            char *key = (char*)(buf + i);
            char *val = (char*)(buf + i + 1);
            db->Put(WriteOptions(), Slice(key, 64), Slice(val, 64));
        }
    }
    printf("Write time (Put): %f\n", c.Stop());
    db->Close();
    DestroyDB(db_name, Options());
}

void test_batch() {
    Clock c;
    DB *db;
    db = open();
    c.Start();
    for(int l=0; l<L; l++) {
        c.Pause();
        generate_data(l);
        c.Restart();
        for(int i=0; i+B <= N; i+=B) {
            WriteBatch batch;
            for(int k=0; k<B; k++){
                char *key = (char*)(buf + i + k);
                char *val = (char*)(buf + i + k + 1);
                batch.Put(Slice(key, 64), Slice(val, 64));
            }
            db->Write(WriteOptions(), &batch);
        }
    }
    printf("Write time (batch): %f\n", c.Stop());
    db->Close();
    DestroyDB(db_name, Options());
}

void test_merged() {
    Clock c;
    DB *db;
    db = open();
    c.Start();
    for(int l=0; l<L; l++) {
        c.Pause();
        generate_data(l);
        c.Restart();
        for(int i=0; i+B <= N; i+=B) {
            char *key = (char*)(&i);
            char *val = (char*)(buf + i);
            db->Put(WriteOptions(), Slice(key, 4), Slice(val, 200 * 8));
        }
    }
    printf("Write time (merged): %f\n", c.Stop());
    db->Close();
    DestroyDB(db_name, Options());
}


int main(int argc, char** argv) {
    parse_args(argc, argv);
    test_puts();
    test_batch();
    test_merged();
    return 0;
}

/*
kvgroup@node1:~/ldeng/SketchStorage$ ./build/apps/rocksdb_batch_test -n 65536
Open: 72571
Writes (single pairs): 284497758
Open: 69138
Writes (batch): 278311831
Open: 48001
Writes (merged): 4802547
kvgroup@node1:~/ldeng/SketchStorage$ ./build/apps/rocksdb_batch_test -n 32768
Open: 63225
Writes (single pairs): 136574943
Open: 65897
Writes (batch): 139723928
Open: 71001
Writes (merged): 1812944
kvgroup@node1:~/ldeng/SketchStorage$ ./build/apps/rocksdb_batch_test -n 16384
Open: 69215
Writes (single pairs): 64999144
Open: 68270
Writes (batch): 60606512
Open: 69996
Writes (merged): 830230
kvgroup@node1:~/ldeng/SketchStorage$ ./build/apps/rocksdb_batch_test -n 8192
Open: 70676
Writes (single pairs): 33669928
Open: 65764
Writes (batch): 27318469
Open: 69581
Writes (merged): 416460
kvgroup@node1:~/ldeng/SketchStorage$ ./build/apps/rocksdb_batch_test -n 4096
Open: 70896
Writes (single pairs): 15901629
Open: 67004
Writes (batch): 13377608
Open: 66559
Writes (merged): 195403
kvgroup@node1:~/ldeng/SketchStorage$ ./build/apps/rocksdb_batch_test -n 2048
Open: 70637
Writes (single pairs): 8150481
Open: 66425
Writes (batch): 6779597
Open: 64365
Writes (merged): 101927
kvgroup@node1:~/ldeng/SketchStorage$ ./build/apps/rocksdb_batch_test -n 1024
Open: 69722
Writes (single pairs): 4000567
Open: 66932
Writes (batch): 3241616
Open: 65762
Writes (merged): 66632
kvgroup@node1:~/ldeng/SketchStorage$ ./build/apps/rocksdb_batch_test -n 512
Open: 70858
Writes (single pairs): 2009757
Open: 197896
Writes (batch): 1574624
Open: 67284
Writes (merged): 30260
*/