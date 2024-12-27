//
// Created by Alfred on 2022/9/11.
//

#ifndef CORANA_ECCACHE_H
#define CORANA_ECCACHE_H

#include "config.h"
#include "ErasureCode/ErasureCode.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

namespace eccache {
    void init(const ConfigParameter& cp, const workload_type& wt);
    void distribution();
    void test(const ConfigParameter& cp, const int& snum);

    static void *twitter_query_exec(void* param);
    static void *meta_query_exec(void* param);
    static void *ibm_query_exec(void* param);
    void ibm_distribution();
    void twitter_distribution();
};


#endif //CORANA_ECCACHE_H
