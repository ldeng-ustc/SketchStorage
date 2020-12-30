#ifndef __PROFILING_H_
#define __PROFILING_H_
#include <unistd.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

//////////////////////////////////////////////////////////////////////////////
//
// process_mem_usage(double &, double &) - takes two doubles by reference,
// attempts to read the system-dependent data for a process' virtual memory
// size and resident set size, and return the results in KB.
//
// On failure, returns 0.0, 0.0
void process_mem_usage(double * vm_usage, double * resident_set);

class Clock {
    std::chrono::steady_clock::time_point start_;
    std::chrono::duration<double> tmp_;
    
public:
    // Start clock and clear.
    void Start();
    // Pause clock.
    double Pause();
    // Restart clock.
    void Restart();
    // Stop clock and return duration seconds. 
    double Stop();
};

#endif