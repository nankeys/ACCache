//
// Created by Alfred on 2022/9/13.
//

#ifndef CORANA_REPLICAS_H
#define CORANA_REPLICAS_H

#include <string>
#include "config.h"
#include <vector>

using namespace std;

class replicas {
public:
    vector<key_param> keys;
    map<string, vector<string>> key_record;

    replicas(ConfigParameter cp, workload_type wt);
    void workload_init();

};


#endif //CORANA_REPLICAS_H
