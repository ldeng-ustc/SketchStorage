#ifndef __UTILITY_H_
#define __UTILITY_H_

#include <sys/time.h>
#include <rocksdb/slice.h>

namespace sketchstorage {
    double ConvertTimevalToDouble(timeval ts) {
        return static_cast<double>(ts.tv_sec) + (ts.tv_usec / 1e6) ;
    }

    timeval ConvertDoubleToTimeval(double ts) {
        timeval tv;
        tv.tv_sec = static_cast<time_t>(ts);
        tv.tv_usec = static_cast<time_t>(ts - tv.tv_sec);
        return tv;
    }

    uint64_t GetEpochId(timeval ts, double epoch_time=0.001) {
        uint64_t epoch_id = static_cast<uint64_t>(ts.tv_sec / epoch_time);
        double epoch_time_us = epoch_time * 1e6;
        epoch_id += static_cast<uint64_t>(ts.tv_usec / epoch_time_us);
        return epoch_id; 
    }

    uint64_t GetEpochId(double ts, double epoch_time=0.001) {
        return GetEpochId(ConvertDoubleToTimeval(ts), epoch_time);
    }

    int cmp_timeval(timeval a, timeval b) {
        if(a.tv_sec != b.tv_sec) {
            return a.tv_sec - b.tv_sec;
        }
        else {
            return a.tv_usec - b.tv_usec;
        }
    }

    rocksdb::Slice ConvertTimevalToSlice(timeval ts) {
        return rocksdb::Slice(reinterpret_cast<const char *>(&ts), sizeof(ts));
    }

    timeval ConvertSliceToTimeval(rocksdb::Slice slice) {
        return *reinterpret_cast<const timeval *>(slice.data());
    }
};

#endif