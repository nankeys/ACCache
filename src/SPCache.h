//
// Created by Alfred on 2022/11/9.
//

#ifndef CORANA_SPCACHE_H
#define CORANA_SPCACHE_H

#include "config.h"

namespace SPCache {
    void initial(workload_type wt, const int& snum);
    void distribution();
    void test(const int& snum);
};


#endif //CORANA_SPCACHE_H
