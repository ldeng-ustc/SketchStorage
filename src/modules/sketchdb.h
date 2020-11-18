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
    class TimevalComparator : public rocksdb::Comparator {
    public:
        int Compare(const rocksdb::Slice& a, const rocksdb::Slice& b) const;

        // Ignore the following methods for now:
        const char* Name() const;
        void FindShortestSeparator(std::string*, const rocksdb::Slice&) const;
        void FindShortSuccessor(std::string*) const;
    };

    class SketchDB {
    public :
        SketchDB(const char *dbfilename);

        bool PutFlowset(const timeval ts, const std::vector<Flow> & flows_list);
        bool PutFlowset_Fake(const timeval ts, const std::vector<Flow> & flows_list);
        bool PutFlowset_WithoutSwitch(const timeval ts, const std::vector<Flow> & flows_list);
        bool Get(const timeval ts, std::vector<Flow> * result);
        bool Scan(const timeval ts_start, const timeval ts_end, std::vector<Flow> * result);
        bool GetFlow(Flowkey5Tuple key, 
            timeval st, timeval ed, std::vector<FlowInfo> * result);
        ~SketchDB();
        void printStats();
        void Close();
    private:
        typedef FlowInfo CounterType;
        typedef Flowkey5Tuple Flowkey ;
        typedef std::vector<std::pair<timeval, CounterType>> CounterList;
        typedef std::unordered_map<Flowkey5Tuple, CounterList, FlowHash> HashTable;
        rocksdb::DB *db_;
        timeval index_timestamp_;
        HashTable * flows_index_;
        HashTable * flows_index_old_;
    };
}

#endif