#include "sketchdb.h"

using namespace std;
using namespace sketchstorage;
namespace sketchstorage
{
    int TimevalComparator::Compare(const rocksdb::Slice& a, const rocksdb::Slice& b) const {
        timeval ta = SliceToTimeval(a);
        timeval tb = SliceToTimeval(b);
        return CmpTimeval(ta, tb);
    }

    const char* TimevalComparator::Name() const {
        return "TimevalComparator";
    }

    void TimevalComparator::FindShortestSeparator(std::string*, const rocksdb::Slice&) const { }
    void TimevalComparator::FindShortSuccessor(std::string*) const { }


    SketchDB::SketchDB(const char *dbfilename)
    {
        rocksdb::Options options;

        TimevalComparator * cmp = new TimevalComparator();
        options.create_if_missing = true;
        options.comparator = cmp;
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

        rocksdb::Status status = rocksdb::DB::Open(options, string(dbfilename), &db_);
        if(!status.ok())
        {
            cerr << "can't open rocksdb" << endl;
            cerr << status.ToString() << endl;
            exit(1);
        }

        flows_index_ = new HashTable();
        flows_index_old_ = new HashTable();
        flows_index_->reserve(300 * 1000);
        index_timestamp_ = {0, 0};
    }

    bool SketchDB::Get(const timeval ts, std::vector<Flow> * result) {
        rocksdb::ReadOptions options(false, true); //verify, cache
        rocksdb::Slice key = TimevalToSlice(ts);
        string value;
        rocksdb::Status s = db_->Get(options, key, &value);
        if(s.IsNotFound()) {
            return false;
        }
        if(!s.ok()) {
            cerr << "read error!" << endl;
            cerr << s.ToString() << endl;
            exit(1);
        }

        const Flow * flows_list = reinterpret_cast<const Flow *>(value.data());
        int flow_cnt = value.size() / sizeof(Flow);
        for(int i = 0; i < flow_cnt; i++) {
            result->push_back(flows_list[i]);
        }
        return true;
    }

    void SketchDB::Close() {
        db_->Close();
    }

    void SketchDB::printStats() {
        std::string stat_str;
        db_->GetProperty("rocksdb.stats",&stat_str);
        cout<< stat_str <<endl;
    }

    bool SketchDB::PutFlowset(const timeval ts, const vector<Flow> & flows_list) {
        auto now_second = GetEpochId(ts, 1);
        if(CmpTimeval(index_timestamp_, now_second) != 0) {
            delete flows_index_old_;
            flows_index_old_ = flows_index_;
            flows_index_ = new HashTable();
            flows_index_->reserve(300 * 1000);
            index_timestamp_ = now_second;
        }
        for(int i = 0; i < flows_list.size(); i++) {
            (*flows_index_)[flows_list[i].flowkey].push_back({ts, flows_list[i].flowinfo});
        }
        rocksdb::Status s;
        rocksdb::Slice key;
        rocksdb::Slice val;
        key = TimevalToSlice(ts);
        val = rocksdb::Slice(reinterpret_cast<const char *>(flows_list.data()), flows_list.size() * sizeof(Flow)); 
        s = db_->Put(rocksdb::WriteOptions(), key, val);
        if(!s.ok())
        {
            cerr << "insert error" << endl;
            cerr << s.ToString() << endl;
            return false;
        }
        return true;
    }

    bool SketchDB::PutFlowset_Fake(const timeval ts, const vector<Flow> & flows_list) {
        rocksdb::Status s;
        rocksdb::Slice key;
        rocksdb::Slice val;
        key = TimevalToSlice(ts);
        val = rocksdb::Slice(reinterpret_cast<const char *>(flows_list.data()), flows_list.size() * sizeof(Flow)); 
        s = db_->Put(rocksdb::WriteOptions(), key, val);
        if(!s.ok())
        {
            cerr << "insert error" << endl;
            cerr << s.ToString() << endl;
            return false;
        }
        return true;
    }

    bool SketchDB::PutFlowset_WithoutSwitch(const timeval ts, const std::vector<Flow> & flows_list) {
        auto now_second = GetEpochId(ts, 1);
        for(int i = 0; i < flows_list.size(); i++) {
            (*flows_index_)[flows_list[i].flowkey].push_back({ts, flows_list[i].flowinfo});
        }
        rocksdb::Status s;
        rocksdb::Slice key;
        rocksdb::Slice val;
        key = TimevalToSlice(ts);
        val = rocksdb::Slice(reinterpret_cast<const char *>(flows_list.data()), flows_list.size() * sizeof(Flow)); 
        s = db_->Put(rocksdb::WriteOptions(), key, val);
        if(!s.ok())
        {
            cerr << "insert error" << endl;
            cerr << s.ToString() << endl;
            return false;
        }
        return true;       
    }

    bool SketchDB::Scan(const timeval ts_start, const timeval ts_end, std::vector<Flow> * result) {
        unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_table;
        rocksdb::ReadOptions options(false, true); //verify, cache
        rocksdb::Iterator* iter = db_->NewIterator(options);
        rocksdb::Slice key_start;
        rocksdb::Slice key_end;
        key_start = TimevalToSlice(ts_start);
        key_end = TimevalToSlice(ts_end);
        iter->Seek(key_start);
        while(iter->Valid() && iter->key().compare(key_end) < 0) {
            const Flow * flow_list = reinterpret_cast<const Flow *>(iter->value().data());
            int flow_cnt = iter->value().size() / sizeof(Flow);
            //printf("ts: %f size: %lu\n", TimevalToDouble(SliceToTimeval(iter->key())), iter->value().size() / sizeof(Flow));
            for(int i = 0; i < flow_cnt; i++) {
                Flow flow = flow_list[i];
                flows_table[flow.flowkey].Merge(flow.flowinfo);
            }
            iter->Next();
        }
        
        for(auto flow_pair: flows_table) {
            result->push_back(Flow(flow_pair));
        }
        delete iter;
        return true;
    }

    bool SketchDB::GetFlow(Flowkey5Tuple key, timeval st, 
                            timeval ed, std::vector<FlowInfo> * result) {
        rocksdb::ReadOptions options(false, true); //verify, cache
        if(flows_index_old_->count(key)) {
            for(auto pair_ts_counter: (*flows_index_old_)[key]) {
                timeval ts = pair_ts_counter.first;
                CounterType counter = pair_ts_counter.second;
                if(CmpTimeval(st, ts) <= 0 && CmpTimeval(ts, ed) <= 0) {
                    result->push_back(counter);
                }
            }
        }
        if(flows_index_->count(key)) {
            for(auto pair_ts_counter: (*flows_index_)[key]) {
                timeval ts = pair_ts_counter.first;
                CounterType counter = pair_ts_counter.second;
                if(CmpTimeval(st, ts) <= 0 && CmpTimeval(ts, ed) <= 0) {
                    result->push_back(counter);
                }
            }
        }
        return true;
    }

    SketchDB::~SketchDB()
    {
        db_->Close();
        delete db_;
    }
}