#ifndef __UTILITY_H_
#define __UTILITY_H_

#include <sys/time.h>
#include <rocksdb/slice.h>

namespace sketchstorage {
    double TimevalToDouble(timeval ts);
    timeval DoubleToTimeval(double ts);
    uint64_t TimevalToLong(timeval ts);
    timeval LongToTimeval(uint64_t ts);
    timeval GetEpochId(timeval ts, double epoch_time=0.001);
    timeval GetEpochId(double ts, double epoch_time=0.001);
    timeval GetEpochId(uint64_t ts, double epoch_time=0.001);
    int CmpTimeval(timeval a, timeval b);
    rocksdb::Slice TimevalToSlice(const timeval & ts);
    timeval SliceToTimeval(rocksdb::Slice slice);
};

#endif