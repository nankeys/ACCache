//
// Created by Alfred on 2022/9/14.
//

#ifndef CORANA_OURSCHEME_H
#define CORANA_OURSCHEME_H

#include "config.h"
//#include "CMSketch.h"
#include "FreqTable.h"
#include "buffer.h"
#include <set>

class OurScheme {
private:
    void getFreqKeys(const string& stats_file);
    vector<string>* readStream(const string& fname, int lnum, long long& cpos) const;
    int isFreqKeys(const string& key);
    vector<int>* group_stat(const string& filename, const int& group_num);
private:
    enum workload_type wtype;
    vector<int> gstat;
    vector<int> gsize;
    size_t total_freq;

public:
    //CMSketch ftable;
    FreqTable ftable;

public:
    explicit OurScheme(enum workload_type wt = twitter);
    void CorrelationAnalysis();
    void distribute(const string& group_file, const int& group_num, const int& node_num);
    //void MemcachedInit();
    void query();
    //void clean();
    ~OurScheme();
    void test();
};


#endif //CORANA_OURSCHEME_H
