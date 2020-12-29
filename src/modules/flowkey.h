#ifndef __FLOWKEY_H_
#define __FLOWKEY_H_

#include <random>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <arpa/inet.h>

struct Flowkey5Tuple {
    // 8 (4*2) bytes
    uint32_t src_ip;  // source IP address
    uint32_t dst_ip;
	// 4 (2*2) bytes
    uint16_t src_port;
    uint16_t dst_port;
    // 1 bytes
    uint8_t proto;
    
    bool operator < (const Flowkey5Tuple & b) const;
    bool operator <= (const Flowkey5Tuple & b) const;
    bool operator > (const Flowkey5Tuple & b) const;
    bool operator >= (const Flowkey5Tuple & b) const;
    bool operator == (const Flowkey5Tuple & b) const;
    bool operator != (const Flowkey5Tuple & b) const;
    Flowkey5Tuple operator ^= (const Flowkey5Tuple & b);
    void Print(FILE * file) const;
    void Print() const;

    static Flowkey5Tuple random(std::minstd_rand & gen);
};

#endif