#include <cstring>
#include<iostream>
#include <vector>
#include <rocksdb/table.h>
#include <rocksdb/cache.h>
#include <rocksdb/slice.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/db.h>
#include <rocksdb/env.h>
#include <rocksdb/filter_policy.h>
#include <rocksdb/statistics.h>

using namespace std;

namespace ycsbc
{

    class RocksDB {
    public :
        RocksDB(const char *dbfilename);

        bool Read(const string &key);

        bool Scan(const std::string &key, int len);

        bool Update(const std::string &key, const string &value);

        bool Insert(const std::string &key, const string &value);

        bool Delete(const std::string &key);
        
        ~RocksDB();
        void printStats();
        void Close();
    private:
        rocksdb::DB *db_;
    };

    RocksDB::RocksDB(const char *dbfilename)
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
            cerr << "can't open RocksDB" << endl;
            cerr << status.ToString() << endl;
            exit(1);
        }

    }

    bool RocksDB::Read(const string &key)
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

    bool RocksDB::Insert(const string &key, const string &value)
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

    bool RocksDB::Delete(const string &key)
    {
        string value;
        return Insert(key, value);
    }

    bool RocksDB::Scan(const string &key, int len)
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

    bool RocksDB::Update(const string &key, const string &value)
    {
        return Insert(key, value);
    }

    void RocksDB::Close()
    {
    }

    void RocksDB::printStats()
    {
        std::string stat_str;
        db_->GetProperty("rocksdb.stats",&stat_str);
        cout<< stat_str <<endl;
    }

    RocksDB::~RocksDB()
    {
        delete db_;
    }
}

int main() {
    ycsbc::RocksDB rdb("./test");

    if (rdb.Insert("abc", "value")) {
        cout << "test Insert OK" << endl;
    }
    else {
        cout << "test Insert Failure" << endl;
    }
    
    if (rdb.Read("abc")) {
        cout << "test Read OK" << endl;
    }
    else {
       cout << "test Read Failure" << endl; 
    }

    return 0;
}
