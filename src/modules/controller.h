#ifndef __CONTROLLER_H_
#define __CONTROLLER_H_

#include "flowradar.h"
#include "double_hashing.h"

class Controller {
public:
    
    Controller(/* args */);
    ~Controller();

    static int decode(
        CountingTable table, 
        std::vector<std::pair<Flowkey5Tuple, FlowInfo>> & out
    );

    static int collaborative_decode(
        CountingTable table,
        std::vector<Flowkey5Tuple> known_keys,
        std::vector<std::pair<Flowkey5Tuple, FlowInfo>> & out
    );
};

#endif
