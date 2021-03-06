/*
 *  Abbreviations and acronyms：
 *      pkt     packet
 *      ts      timestamp
 *      hdr     header
 */

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <climits>

#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap/pcap.h>

#include "./modules/flow.h"
#include "./modules/trace.h"

using namespace std;

typedef Flowkey5Tuple Flowkey;

map<Flowkey, FlowInfo> flowmap;
vector<FlowInfo> flow_list;

const char DATA_PATH[] = "/data/caida/trace.bin";
char errmsg[PCAP_ERRBUF_SIZE];

vector<int> count_active_flows(const map<Flowkey, FlowInfo> & flowmap, double interval) {
    map<uint64_t, int> delta;
    for(auto flow: flowmap) {
        uint64_t l = (uint64_t)(flow.second.start_time_ / interval);
        uint64_t r = (uint64_t)(flow.second.end_time_ / interval);
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

vector<int> count_unique_flows(Trace trace, double interval) {
    const vector<PacketInfo> & pktlist = trace.packet_list;
    vector<int> cnt_list;
    uint64_t ts = (uint64_t)(pktlist[0].ts / interval);
    int cnt = 0;
    set<Flowkey> flowset;
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
    Trace trace;
    int st = clock();
    trace.load(DATA_PATH);
    printf("%lf\n", (clock() - st) / (double)(CLOCKS_PER_SEC));
    for(auto pkt: trace.packet_list) {
        const Flowkey5Tuple & key = pkt.key;
        const double & pkt_ts = pkt.ts; 

        if(flowmap[key].pkt_cnt_ == 0) {
            flowmap[key].start_time_ = pkt_ts;
        }
        flowmap[key].end_time_ = pkt_ts;
        flowmap[key].pkt_cnt_ ++;
        flowmap[key].flow_size_ += pkt.size;
    }

    double maxtime = 0;
    double mintime = 1e100;
    double avgtime = 0;
    for(auto flow: flowmap) {
        double flowtime = flow.second.end_time_ - flow.second.start_time_;
        maxtime = max(maxtime, flowtime);
        mintime = min(mintime, flowtime);
        avgtime += flowtime;
    }
    avgtime /= flowmap.size();

    printf(
        "total_packets: %u\nstart_time: %lf\nend_time: %lf\n",
        trace.pkt_cnt(),
        trace.start_time(), 
        trace.end_time()
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

    FILE *file = nullptr;
    mkdir("./data", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir("./data/analysis", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
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
        vector<int> && cnt_list = count_unique_flows(trace, interval);
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

    
    file = fopen("./data/analysis/flows.csv", "w");
    int i = 0;
    int single_pkt_flow_cnt = 0;
    fprintf(file, "flow_id,start,end,duration,packet_cnt,size\n");
    for(auto flow: flow_list) {
        if(flow.pkt_cnt_ == 1){
            single_pkt_flow_cnt ++;
        }
        else{
            fprintf(
                file,
                "%d,%lf,%lf,%lf,%u,%u\n",
                ++i,
                flow.start_time_,
                flow.end_time_,
                flow.end_time_ - flow.start_time_,
                flow.pkt_cnt_,
                flow.flow_size_ 
            );
        }
    }
    fclose(file);
    printf("single packet flows: %d\n", single_pkt_flow_cnt);
    

    file = fopen("./data/analysis/flows_duration_distribution.csv", "w");
    sort(flow_list.begin(), flow_list.end(), FlowInfo::lt_duration);
    for(int i=0; i<100; i++) {
        int index = (int)(i / 100.0 * flow_list.size());
        double duration = flow_list[index].end_time_ - flow_list[index].start_time_;
        fprintf(file, "%lf,%lf\n", i/100.0, duration);
    }
    fclose(file);

    file = fopen("./data/analysis/flows_size_distribution.csv", "w");
    sort(flow_list.begin(), flow_list.end(), FlowInfo::lt_size);
    for(int i=0; i<100; i++) {
        int index = (int)(i / 100.0 * flow_list.size());
        double size = flow_list[index].flow_size_;
        fprintf(file, "%lf,%lf\n", i/100.0, size);
    }
    fclose(file);

    file = fopen("./data/analysis/flows_pktcnt_distribution.csv", "w");
    sort(flow_list.begin(), flow_list.end(), FlowInfo::lt_pkt_cnt);
    for(int i=0; i<100; i++) {
        int index = (int)(i / 100.0 * flow_list.size());
        double pktcnt = flow_list[index].pkt_cnt_;
        fprintf(file, "%lf,%lf\n", i/100.0, pktcnt);
    }
    fclose(file);

    return 0;
}

