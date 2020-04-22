#include "sketchdb.h"

using namespace std;
using namespace sketchstorage;
namespace sketchstorage
{
    SketchDB::SketchDB(const char *dbfilename)
    {
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

        rocksdb::Status status = rocksdb::DB::Open(options, string(dbfilename), &db_);
        if(!status.ok())
        {
            cerr << "can't open rocksdb" << endl;
            cerr << status.ToString() << endl;
            exit(1);
        }

    }

    bool SketchDB::Read(const string &key)
    {
        rocksdb::ReadOptions options(false, true); //verify, cache

        std::string value;
        rocksdb::Slice key_(key);
        rocksdb::Status s = db_->Get(options, key_, &value);
        if(s.IsNotFound())
        {
            return false;
        }
        
        if(!s.ok())
        {
            cerr << "read error!" << endl;
            cerr << s.ToString() << endl;
            exit(1);
        }

        return true;
    }

    bool SketchDB::Insert(const string &key, const string &value)
    {
        rocksdb::Status s;
        s = db_->Put(rocksdb::WriteOptions(), rocksdb::Slice(key), rocksdb::Slice(value));
        if(!s.ok())
        {
            cerr << "insert error" << endl;
            cerr << s.ToString() << endl;
            exit(1);
        }
        
        return true;
    }

    bool SketchDB::Delete(const string &key)
    {
        string value;
        return Insert(key, value);
    }

    bool SketchDB::Scan(const string &key, int len)
    {
        rocksdb::ReadOptions options(false, true); //verify, cache
        rocksdb::Iterator* iter = db_->NewIterator(options);
        iter->Seek(key);
        for (int i = 0; i < len; i++) {
            // iter->key();
            // iter->value();
            iter->Next();
        }
        delete iter;
        return true;
    }

    bool SketchDB::Update(const string &key, const string &value)
    {
        return Insert(key, value);
    }

    void SketchDB::Close()
    {
    }

    void SketchDB::printStats()
    {
        std::string stat_str;
        db_->GetProperty("rocksdb.stats",&stat_str);
        cout<< stat_str <<endl;
    }

    bool SketchDB::Insert(const timeval ts, const vector<Flow> & flows) {
        for(int i = 0; i < flows.size(); i++) {
            flows_index_[flows[i].flowkey].push_back(ts);
        }
        rocksdb::Status s;
        rocksdb::Slice key;
        rocksdb::Slice val;
        key = ConvertTimevalToSlice(ts);
        val = rocksdb::Slice(reinterpret_cast<const char *>(flows.data()), flows.size() * sizeof(Flow)); 
        s = db_->Put(rocksdb::WriteOptions(), key, val);
        if(!s.ok())
        {
            cerr << "insert error" << endl;
            cerr << s.ToString() << endl;
            exit(1);
        }
        return true;
    }

    bool SketchDB::Scan(const timeval ts_start, const timeval ts_end, std::vector<Flow> * result) {
        unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_table;
        rocksdb::ReadOptions options(false, true); //verify, cache
        rocksdb::Iterator* iter = db_->NewIterator(options);
        rocksdb::Slice key_start;
        rocksdb::Slice key_end;
        key_start = ConvertTimevalToSlice(ts_start);
        key_end = ConvertTimevalToSlice(ts_end);
        iter->Seek(key_start);
        while(iter->Valid() && iter->key().compare(key_end) < 0) {
            const Flow * flow_list = reinterpret_cast<const Flow *>(iter->value().data());
            int flow_cnt = iter->value().size() / sizeof(Flow);
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

    bool SketchDB::Read(const Flowkey5Tuple key, std::vector<FlowInfo> * result) {
        rocksdb::ReadOptions options(false, true); //verify, cache

        for(timeval ts: flows_index_[key]) {
            rocksdb::Slice key_slice = ConvertTimevalToSlice(ts);
            string value;
            rocksdb::Status s = db_ -> Get(options, key_slice, &value);
            if(!s.ok()) {
                cerr << "read error!" << endl;
                cerr << s.ToString() << endl;
                exit(1);
            }
            if(s.IsNotFound()) {
                return false;
            }
            int num_flows = value.size() / sizeof(Flow);
            const Flow * flows = reinterpret_cast<const Flow *>(value.data()); 
            for(int i = 0; i < num_flows; i++) {
                if(flows[i].flowkey == key) {
                    result->push_back(flows[i].flowinfo);
                    break;
                }
            }
        }
        return true;
    }

    SketchDB::~SketchDB()
    {
        delete db_;
    }
}