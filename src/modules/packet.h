#ifndef __PACKET_H_
#define __PACKET_H_

#include <random>
#include <cstdint>

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap/pcap.h>

#include "flow.h"

struct PacketInfo {
    Flowkey5Tuple key;
    uint32_t size;
    double ts;
    static PacketInfo random(std::minstd_rand & gen);
};

int decode_packet(const pcap_pkthdr & pcap_hdr, const uint8_t *packet, PacketInfo & pkt_info);

#endif