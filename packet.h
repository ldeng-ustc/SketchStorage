#ifndef __PACKET_H_
#define __PACKET_H_

#include <cstdint>

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap/pcap.h>

#include "flow.h"

struct packet_info_t {
    flowkey_5_tuple_t key;
    uint32_t size;
    double ts;
};

int decode_packet(const pcap_pkthdr & pcap_hdr, const uint8_t *packet, packet_info_t & pkt_info) {
    double pkt_ts = (double)pcap_hdr.ts.tv_usec / 1000000 + pcap_hdr.ts.tv_sec;
    uint32_t pkt_len = pcap_hdr.caplen;
    flowkey_t key;

    ip* ip_hdr;
    tcphdr* tcp_hdr;
    udphdr* udp_hdr;
    
    ip_hdr = (ip*)(packet);
    if ((int)pkt_len < (ip_hdr->ip_hl << 2)) {
        return -1;
    }
    if (ip_hdr->ip_v != 4) {
        return -1;
    }
    
    if (ip_hdr->ip_p == IPPROTO_TCP) {
        // see if the TCP header is fully captured
        tcp_hdr = (struct tcphdr*)((uint8_t*)ip_hdr + (ip_hdr->ip_hl << 2));
        if ((int)pkt_len < (ip_hdr->ip_hl << 2) + (tcp_hdr->doff << 2)) {
            return -1;
        }
    } else if (ip_hdr->ip_p == IPPROTO_UDP) {
        // see if the UDP header is fully captured
        if ((int)pkt_len < (ip_hdr->ip_hl << 2) + 8) {
            return -1;
        }
    } else if (ip_hdr->ip_p == IPPROTO_ICMP) {
        // see if the ICMP header is fully captured
        if ((int)pkt_len < (ip_hdr->ip_hl << 2) + 8) {
            return -1;
        }
    }

    key.src_ip = ip_hdr->ip_src.s_addr;
    key.dst_ip = ip_hdr->ip_dst.s_addr;
    key.proto = ip_hdr->ip_p;
    if (ip_hdr->ip_p == IPPROTO_TCP) {
        // TCP
        tcp_hdr = (struct tcphdr*)((uint8_t*)ip_hdr + (ip_hdr->ip_hl << 2));
        key.src_port = ntohs(tcp_hdr->source);
        key.dst_port = ntohs(tcp_hdr->dest);
    }
    else if (ip_hdr->ip_p == IPPROTO_UDP) {
        // UDP
        udp_hdr = (struct udphdr*)((uint8_t*)ip_hdr + (ip_hdr->ip_hl << 2));
        key.src_port = ntohs(udp_hdr->source);
        key.dst_port = ntohs(udp_hdr->dest);
    } else {
        // Other L4
        key.src_port = 0;
        key.dst_port = 0;
    }

    pkt_info = {key, ntohs(ip_hdr->ip_len), pkt_ts};

    return 0;
}

#endif