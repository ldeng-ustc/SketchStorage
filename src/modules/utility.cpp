#include "utility.h"
namespace sketchstorage {
    double TimevalToDouble(const timeval & ts) {
        return static_cast<double>(ts.tv_sec) + (ts.tv_usec / 1e6) ;
    }

    timeval DoubleToTimeval(double ts) {
        timeval tv;
        tv.tv_sec = static_cast<time_t>(ts);
        tv.tv_usec = static_cast<time_t>((ts - tv.tv_sec) * 1e6);
        return tv;
    }

    uint64_t TimevalToLong(const timeval & ts) {
        return (ts.tv_sec * 1000000) + ts.tv_usec;
    }

    timeval LongToTimeval(uint64_t ts) {
        return {static_cast<time_t>(ts / 1000000), static_cast<suseconds_t>(ts % 1000000)};
    }

    timeval GetEpochId(const timeval & ts, double epoch_time) {
        uint64_t epoch_time_us = static_cast<uint64_t>(1e6 * epoch_time);
        uint64_t long_ts = TimevalToLong(ts);
        return LongToTimeval(long_ts - long_ts % epoch_time_us); 
    }

    timeval GetEpochId(double ts, double epoch_time) {
        return GetEpochId(DoubleToTimeval(ts), epoch_time);
    }

    timeval GetEpochId(uint64_t ts, double epoch_time) {
        return GetEpochId(LongToTimeval(ts), epoch_time);
    }

    int CmpTimeval(const timeval & a, const timeval & b) {
        if(a.tv_sec != b.tv_sec) {
            return a.tv_sec - b.tv_sec;
        }
        else {
            return a.tv_usec - b.tv_usec;
        }
    }

    rocksdb::Slice TimevalToSlice(const timeval & ts) {
        return rocksdb::Slice(reinterpret_cast<const char *>(&ts), sizeof(ts));
    }

    timeval SliceToTimeval(const rocksdb::Slice & slice) {
        return *reinterpret_cast<const timeval *>(slice.data());
    }
}