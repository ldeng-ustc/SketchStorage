#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include <cstdio>
#include <cstring>
#include <climits>

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap/pcap.h>

#include "flow.h"
#include "trace.h"

using namespace std;



map<flowkey_t, flowinfo_t> flowmap;
vector<packetinfo_t> packet_list;
vector<flowinfo_t> flow_list;

trace_info_t trace_info;

const char DATA_PATH[] = "/data/caida/equinix-nyc.dirA.20190117-130000.UTC.anon.pcap";
char errmsg[PCAP_ERRBUF_SIZE];

vector<int> count_active_flows(const map<flowkey_t, flowinfo_t> & flowmap, double interval) {
    map<uint64_t, int> delta;
    for(auto flow: flowmap) {
        uint64_t l = (uint64_t)(flow.second.start_time / interval);
        uint64_t r = (uint64_t)(flow.second.end_time / interval);
        delta[l] ++;
        delta[r + 1] --;
    }
    uint64_t l = delta.begin()->first;
    uint64_t r = delta.rbegin()->first;

    vector<int> cnt_list;
    int cnt = 0;
    for(uint64_t i=l; i<r; i++) {
        cnt += delta[i];
        cnt_list.push_back(cnt);
    }
    return cnt_list;
}

vector<int> count_unique_flows(const vector<packetinfo_t> & pktlist, double interval) {
    vector<int> cnt_list;
    uint64_t ts = (uint64_t)(pktlist[0].ts / interval);
    int cnt = 0;
    set<flowkey_t> flowset;
    for(auto pkt: pktlist) {
        uint64_t t = (uint64_t)(pkt.ts / interval);
        if(t != ts) {
            cnt_list.push_back(cnt);
            cnt = 0;
            flowset.clear();
            ts = t;
        }
        if(!flowset.count(pkt.key)) {
            cnt++;
            flowset.insert(pkt.key);
        }
    }
    return cnt_list;
}

int main(int argc, char** argv) {
    pcap_t *p = pcap_open_offline(DATA_PATH, errmsg);
    const uint8_t *pkt;
    pcap_pkthdr hdr;
    while((pkt = pcap_next(p, &hdr)) != NULL) {
        double pkt_ts = (double)hdr.ts.tv_usec / 1000000 + hdr.ts.tv_sec;
        uint32_t pkt_len = hdr.caplen;
        flowkey_t key;

        ip* ip_hdr;
        tcphdr* tcp_hdr;
        udphdr* udp_hdr;
        
        ip_hdr = (ip*)(pkt);
        if ((int)pkt_len < (ip_hdr->ip_hl << 2)) {
            continue;
        }
        if (ip_hdr->ip_v != 4) {
            continue;
        }
        
        if (ip_hdr->ip_p == IPPROTO_TCP) {
            // see if the TCP header is fully captured
            tcp_hdr = (struct tcphdr*)((uint8_t*)ip_hdr + (ip_hdr->ip_hl << 2));
            if ((int)pkt_len < (ip_hdr->ip_hl << 2) + (tcp_hdr->doff << 2)) {
                continue;
            }
        } else if (ip_hdr->ip_p == IPPROTO_UDP) {
            // see if the UDP header is fully captured
            if ((int)pkt_len < (ip_hdr->ip_hl << 2) + 8) {
                continue;
            }
        } else if (ip_hdr->ip_p == IPPROTO_ICMP) {
            // see if the ICMP header is fully captured
            if ((int)pkt_len < (ip_hdr->ip_hl << 2) + 8) {
                continue;
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

        if(flowmap[key].pkt_cnt == 0) {
            flowmap[key].start_time = pkt_ts;
        }
        flowmap[key].end_time = pkt_ts;
        flowmap[key].pkt_cnt ++;
        flowmap[key].flow_size += ntohs(ip_hdr->ip_len);
        if(trace_info.pkt_cnt == 0) {
            trace_info.start_time = pkt_ts;
        }
        trace_info.pkt_cnt ++;
        trace_info.end_time = pkt_ts;
        packet_list.push_back({key, ntohs(ip_hdr->ip_len), pkt_ts});
    }

    double maxtime = 0;
    double mintime = 1e100;
    double avgtime = 0;
    for(auto flow: flowmap) {
        double flowtime = flow.second.end_time - flow.second.start_time;
        maxtime = max(maxtime, flowtime);
        mintime = min(mintime, flowtime);
        avgtime += flowtime;
    }
    avgtime /= flowmap.size();

    printf(
        "total_packets: %u\nstart_time: %lf\nend_time: %lf\n",
        trace_info.pkt_cnt,
        trace_info.start_time, 
        trace_info.end_time
    );
    printf("total_flow: %lu\n", flowmap.size());
    printf(
        "duration:\n\tavg: %lf\n\tmax: %lf\n\tmin: %lf\n",
        avgtime,
        maxtime,
        mintime
    );

    for(auto flow: flowmap) {
        flow_list.push_back(flow.second);
    }

    FILE *file;
    /*
    vector<double> interval_list {0.001, 0.01, 0.1, 1, 10};
    for(double interval: interval_list) {
        vector<int> && cnt_list = count_active_flows(flowmap, interval);
        int maxcnt = 0;
        int mincnt = INT_MAX;
        double avgcnt = 0;
        for(int cnt: cnt_list) {
            maxcnt = max(maxcnt, cnt);
            mincnt = min(mincnt, cnt);
            avgcnt += cnt;
        }
        avgcnt /= cnt_list.size();
        printf("active flows count - interval: %lf\n", interval);
        printf(
            "\tavg: %lf\n\tmax: %d\n\tmin: %d\n",
            avgcnt,
            maxcnt,
            mincnt
        );
    }

    for(double interval: interval_list) {
        vector<int> && cnt_list = count_unique_flows(packet_list, interval);
        int maxcnt = 0;
        int mincnt = INT_MAX;
        double avgcnt = 0;
        for(int cnt: cnt_list) {
            maxcnt = max(maxcnt, cnt);
            mincnt = min(mincnt, cnt);
            avgcnt += cnt;
        }
        avgcnt /= cnt_list.size();
        printf("unique flows count - interval: %lf\n", interval);
        printf(
            "\tavg: %lf\n\tmax: %d\n\tmin: %d\n",
            avgcnt,
            maxcnt,
            mincnt
        );
    }

    file = fopen("flows.csv", "w");
    int i = 0;
    int single_pkt_flow_cnt = 0;
    fprintf(file, "flow_id,start,end,duration,packet_cnt,size\n");
    for(auto flow: flow_list) {
        if(flow.second.pkt_cnt == 1){
            single_pkt_flow_cnt ++;
        }
        else{
            fprintf(
                file,
                "%d,%lf,%lf,%lf,%u,%u\n",
                ++i,
                flow.second.start_time,
                flow.second.end_time,
                flow.second.end_time - flow.second.start_time,
                flow.second.pkt_cnt,
                flow.second.flow_size 
            );
        }
        flow_list.push_back(flow.second);
    }
    fclose(file);
    printf("single packet flows: %d\n", single_pkt_flow_cnt);
    */

    file = fopen("flows_duration_distribution.csv", "w");
    sort(flow_list.begin(), flow_list.end(), flowinfo_t::lt_duration);
    for(int i=0; i<100; i++) {
        int index = (int)(i / 100.0 * flow_list.size());
        double duration = flow_list[index].end_time - flow_list[index].start_time;
        fprintf(file, "%lf,%lf\n", i/100.0, duration);
    }
    fclose(file);

    file = fopen("flows_size_distribution.csv", "w");
    sort(flow_list.begin(), flow_list.end(), flowinfo_t::lt_size);
    for(int i=0; i<100; i++) {
        int index = (int)(i / 100.0 * flow_list.size());
        double size = flow_list[index].flow_size;
        fprintf(file, "%lf,%lf\n", i/100.0, size);
    }
    fclose(file);

    file = fopen("flows_pktcnt_distribution.csv", "w");
    sort(flow_list.begin(), flow_list.end(), flowinfo_t::lt_pkt_cnt);
    for(int i=0; i<100; i++) {
        int index = (int)(i / 100.0 * flow_list.size());
        double pktcnt = flow_list[index].pkt_cnt;
        fprintf(file, "%lf,%lf\n", i/100.0, pktcnt);
    }
    fclose(file);

    return 0;
}

