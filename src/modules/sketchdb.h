#ifndef __ROCKSDB_H_
#define __ROCKSDB_H_
#include <cstring>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sys/time.h>
#include <rocksdb/table.h>
#include <rocksdb/cache.h>
#include <rocksdb/slice.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/db.h>
#include <rocksdb/env.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/statistics.h>

#include "flow.h"
#include "utility.h"

namespace sketchstorage
{

    class SketchDB {
    public :
        SketchDB(const char *dbfilename);

        bool Read(const timeval ts, std::vector<Flow> * result);
       
        bool Insert(const timeval ts, const std::vector<Flow> & flow_list);

        bool Scan(const timeval ts_start, const timeval ts_end, std::vector<Flow> * result);

        bool GetDetailInfo(const Flowkey5Tuple key, std::vector<FlowInfo> * result);


        ~SketchDB();
        void printStats();
        void Close();
    private:
        rocksdb::DB *db_;
        std::unordered_map<Flowkey5Tuple, std::vector<timeval>, FlowHash> flows_index_;
    };
}

#endif