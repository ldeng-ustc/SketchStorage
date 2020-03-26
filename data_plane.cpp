#include <vector>
#include <cstdio>
#include <functional>
#include <iostream>
#include <bitset>

#include "packet.h"
#include "flow.h"
#include "trace.h"
#include "flow_radar.h"
#include "MurmurHash3.h"

using namespace std;

int main(int argc, char** argv) {
    //trace_t trace;
    //trace.load("./data/trace.bin");
    cout << sizeof(flowkey_5_tuple_t) << endl;
    cout << sizeof(flow_radar_element_t) << endl;
}