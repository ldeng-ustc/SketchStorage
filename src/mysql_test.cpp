#include <getopt.h>
#include <mysqlx/xdevapi.h>
#include <sys/resource.h>

#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <unordered_map>

#include "modules/flow.h"
#include "modules/profiling.h"
#include "modules/sketchdb.h"
#include "modules/trace.h"

using namespace sketchstorage;
using namespace std;

using mysqlx::Row;
using mysqlx::RowResult;
using mysqlx::Schema;
using mysqlx::Session;
using mysqlx::Table;

const char *Db_Path = "./data/db/test";
const char *Trace_Path = "./data/caida/trace.bin";
const char *Output_Path_Std = "./data/test/true_flows.txt";
const char *Output_Path_Db = "./data/test/db_flows.txt";
const char *Table_Name = "sketch";
string Eval_Data_Dir = "./data/evaluations/mysql/";
int duration_ms = 60000;
int scan_ms = 100;
bool insert_without_index = false;

void parse_args(int argc, char **argv) {
    int c;
    while ((c = getopt(argc, argv, "d:o:s:i:")) != -1) {
        switch (c) {
        case 'd':
            sscanf(optarg, "%d", &duration_ms);
            break;
        case 'o':
            Eval_Data_Dir = string(optarg);
        case 's':
            sscanf(optarg, "%d", &scan_ms);
        case 'i':
            Table_Name = optarg;
        default:
            break;
        }
    }
}

typedef int (*PutFlowsetFunc)(Session *, const timeval &, const vector<Flow> &);

int PutFlowset(Session *db, const timeval &epoch_id,
               const vector<Flow> &flow_list) {
    Schema myDb = db->getSchema("sketch");
    Table myTable = myDb.getTable(Table_Name);

    auto &&inserter = myTable.insert(
        "ts_second", "ts_us",
        "src_ip", "dst_ip", "src_port", "dst_port", "protocol",
        "start_time", "end_time", "pkt_cnt", "flow_size");
    for (Flow flow : flow_list) {
        const Flowkey5Tuple &key = flow.flowkey;
        const FlowInfo &info = flow.flowinfo;
        inserter.values(
            epoch_id.tv_sec, epoch_id.tv_usec,
            key.src_ip, key.dst_ip, key.src_port, key.dst_port, key.proto,
            info.start_time_, info.end_time_, info.pkt_cnt_, info.flow_size_);
    }
    Clock c;
    c.start();
    inserter.execute();
    return c.stop();
}

int Scan(Session *db, timeval st, timeval ed) {
    char sql[1000];
    sprintf(
        sql,
        "SELECT "
        "src_ip, dst_ip, src_port, dst_port, protocol, "
        "min(start_time), max(end_time), sum(pkt_cnt), sum(flow_size) "
        "FROM %s "
        " WHERE "
        "(ts_second > %lu OR (ts_second = %lu AND ts_us >= %lu))"
        " AND "
        "(ts_second < %lu OR (ts_second = %lu AND ts_us <= %lu))"
        " GROUP BY "
        "src_ip, dst_ip, src_port, dst_port, protocol",
        Table_Name,
        st.tv_sec, st.tv_sec, st.tv_usec,
        ed.tv_sec, ed.tv_sec, ed.tv_usec);
    Clock c;
    c.start();
    RowResult sql_result = db->sql(sql).execute();
    int t = c.stop();
    //printf("size: %lu\n", sql_result.count());
    fflush(stdout);
    return t;
}

int GetFlow(Session *db, Flowkey5Tuple key, timeval st, timeval ed, int *result_size) {
    char sql[1000];
    sprintf(
        sql,
        "SELECT * FROM %s WHERE "
        "(ts_second > %lu OR (ts_second = %lu AND ts_us >= %lu))"
        " AND "
        "(ts_second < %lu OR (ts_second = %lu AND ts_us <= %lu))"
        " AND "
        "src_ip = %u AND dst_ip = %u"
        " AND "
        "src_port = %hu AND dst_port = %hu"
        " AND "
        "protocol = %hhu",
        Table_Name,
        st.tv_sec, st.tv_sec, st.tv_usec,
        ed.tv_sec, ed.tv_sec, ed.tv_usec,
        key.src_ip, key.dst_ip,
        key.src_port, key.dst_port,
        key.proto);
    Clock c;
    c.start();
    RowResult sql_result = db->sql(sql).execute();
    int t = c.stop();
    *result_size = sql_result.count();
    //printf("size: %lu\n", sql_result.count());
    //fflush(stdout);
    return t;
}

vector<int> TestDelayPutFlowset(Session *db, PutFlowsetFunc PutFlowset) {
    vector<int> result;
    TraceIterator trace_iter(Trace_Path);
    double trace_start_time = (*trace_iter).ts;

    timeval epoch_id = GetEpochId(trace_start_time);
    timeval duration = {duration_ms / 1000, duration_ms % 1000 * 1000};
    timeval end_epoch;
    timeradd(&epoch_id, &duration, &end_epoch);

    unordered_map<Flowkey5Tuple, FlowInfo, FlowHash> flows_map;
    PacketInfo pkt;

    Clock c;
    while (trace_iter.next(&pkt)) {
        //printf("epoch_id: %d\n", GetEpochId(pkt.ts));
        //fflush(stdout);
        if (CmpTimeval(GetEpochId(pkt.ts), epoch_id) != 0) {
            //printf("epoch: %d\n", epoch_id.tv_usec);
            //fflush(stdout);
            vector<Flow> flows_list;
            for (auto flow_pair : flows_map) {
                flows_list.push_back(Flow(flow_pair));
            }
            flows_map.clear();
            //printf("insert size: %lu\n", flows_list.size());
            int t = PutFlowset(db, epoch_id, flows_list);
            result.push_back(t);
            epoch_id = GetEpochId(pkt.ts);
            if (CmpTimeval(epoch_id, end_epoch) >= 0) {
                break;
            }
        }
        flows_map[pkt.key].AddPacket(pkt);
    }
    return result;
}

vector<int> TestDelayScan(Session *db) {
    vector<int> result;
    TraceIterator trace_iter(Trace_Path);
    double trace_start_time = (*trace_iter).ts;
    timeval begin_ts = DoubleToTimeval(trace_start_time);

    random_device rd;
    minstd_rand gen(0);
    uniform_int_distribution<> r(0, duration_ms - scan_ms);
    Clock c;
    for (int i = 0; i < 1000; i++) {
        int random_ms = r(gen);
        //printf("ms: %d\n", random_ms);
        timeval random_ts = {random_ms / 1000, random_ms % 1000 * 1000};

        timeval scan_start;
        timeval scan_end;
        timeval step = {0, scan_ms * 1000};
        timeradd(&begin_ts, &random_ts, &scan_start);
        timeradd(&scan_start, &step, &scan_end);
        //printf("%lf, %lf, %lf\n", TimevalToDouble(random_ts), TimevalToDouble(scan_start), TimevalToDouble(scan_end));

        vector<Flow> scan_result;
        int t = Scan(db, scan_start, scan_end);
        result.push_back(t);
        //printf("%d\n", scan_result.size());
    }
    return result;
}

vector<int> TestDelayGetFlow(Session *db) {
    vector<int> result;
    TraceIterator trace_iter(Trace_Path);
    PacketInfo pkt;

    random_device rd;
    minstd_rand gen(0);
    uniform_int_distribution<> r(0, 200);
    while (trace_iter.next(&pkt)) {
        if (r(gen) == 0) {
            timeval ts_start = GetEpochId(pkt.ts);
            timeval ts_end;
            timeval ts_range = {1, 0};
            timeradd(&ts_start, &ts_range, &ts_end);
            vector<FlowInfo> getflow_result;
            int ret;
            int t = GetFlow(db, pkt.key, ts_start, ts_end, &ret);
            if (ret == 0) {
                break;
            }
            result.push_back(t);
        }
    }
    return result;
}

void print_result(FILE *file, const vector<int> &result) {
    fprintf(file, "%d", result[0]);
    for (size_t i = 1; i < result.size(); i++) {
        fprintf(file, ",%d", result[i]);
    }
    fprintf(file, "\n");
}

int main(int argc, char **argv) {
    Session mySession(33060, "root", "rootroot");
    Schema myDb = mySession.getSchema("sketch");
    myDb.getTable("sketch").remove().execute();
    myDb.getTable("flow").remove().execute();
    mySession.sql("use sketch").execute();

    parse_args(argc, argv);

    FILE *eval_output;
    vector<int> result;

    //PutFlowset
    string PathDelayPutFlowset = Eval_Data_Dir + "delay_PutFlowset_" + Table_Name + ".csv";
    eval_output = fopen(PathDelayPutFlowset.c_str(), "w");
    result = TestDelayPutFlowset(&mySession, PutFlowset);
    print_result(eval_output, result);
    fclose(eval_output);
    result.clear();

    //Scan
    string PathDelayScan = Eval_Data_Dir + "delay_Scan_" + Table_Name + ".csv";
    eval_output = fopen(PathDelayScan.c_str(), "w");
    result = TestDelayScan(&mySession);
    print_result(eval_output, result);
    fclose(eval_output);
    result.clear();

    string PathDelayGetFlow = Eval_Data_Dir + "delay_GetFlow_" + Table_Name + ".csv";
    eval_output = fopen(PathDelayGetFlow.c_str(), "w");
    result = TestDelayGetFlow(&mySession);
    print_result(eval_output, result);
    // result.clear();
    // result = TestDelayGetFlow(&mySession);
    // print_result(eval_output, result);
    fclose(eval_output);
    result.clear();

    return 0;
}