#ifndef __UTILITY_H_
#define __UTILITY_H_

#include <sys/time.h>
#include <rocksdb/slice.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

namespace sketchstorage {
    double TimevalToDouble(const timeval & ts);
    timeval DoubleToTimeval(double ts);
    uint64_t TimevalToLong(const timeval & ts);
    timeval LongToTimeval(uint64_t ts);
    timeval GetEpochId(const timeval & ts, double epoch_time=0.001);
    timeval GetEpochId(double ts, double epoch_time=0.001);
    timeval GetEpochId(uint64_t ts, double epoch_time=0.001);
    int CmpTimeval(const timeval & a, const timeval & b);
    rocksdb::Slice TimevalToSlice(const timeval & ts);
    timeval SliceToTimeval(const rocksdb::Slice & slice);
};

#endif