//
// Created by Alfred on 2022/7/22.
//

#ifndef CORANA_CONFIG_H
#define CORANA_CONFIG_H

#include <jsoncpp/json/json.h>
#include <fstream>
#include <iostream>
#include <queue>
#include "fmt/core.h"

using namespace std;

enum workload_type{
    twitter = 0,
    meta,
    ibm
};

typedef struct {
    uint32_t tid;
    double runtime;
    //vector<double> latency;
    priority_queue<double,vector<double>, greater<> > latency;
    int ops;
    double thput_of_ops;
    unsigned long long size;
    long double thput_of_size;
} thread_param;

typedef struct k{
    string key;
    size_t size;
    int freq;
    bool operator==(const struct k& ln) const {
        return this->key == ln.key;
    }
}key_param;

typedef struct {
    string ckey;    // chunk key
    size_t offset;
    size_t length;  // value length
    size_t stripe_id;
    short chunk_id;
}agg_key;

static bool key_freq_comp(const key_param &a, const key_param &b) {
    if (a.freq > b.freq) {
        return true;
    } else {
        return false;
    }
}

static bool key_string_comp(const key_param &a, const key_param &b) {
    if (a.key > b.key) {
        return true;
    } else {
        return false;
    }
}

//static void readStat(const string& stats_file, vector<key_param>& keys) {
static vector<key_param> readStat(const string& stats_file) {
    vector<key_param> keys;
    ifstream fin(stats_file);

    if(!fin.is_open()) {
        cout << "Error opening stats file!" << endl;
        exit(-1);
    }

    int i = 0;
    while(fin.peek() != EOF) {
        key_param tmp;
        fin >> tmp.key >> tmp.size >> tmp.freq;
        if(tmp.key.empty()) break;
        keys.push_back(tmp);
        i++;
    }

    cout << "stat line = " << keys.size() << endl;
    cout << "stat line = " << i << endl;

    return keys;
}

static int FreqSearch(vector<key_param>& keys, long start, long end, const string& ss){
    long left = start;
    long right = end - 1;

    int mid = 0;
    //定义域为[left,right]
    while(left <= right) {
        //此时标记left位置，防止下标越界
        mid = left + (right - left) / 2;
        if(keys[mid].key == ss) {
            return mid;
        }
        if(keys[mid].key > ss) {
            left = mid + 1;
        }
        if(keys[mid].key < ss) {
            right = mid - 1;
        }
    }
    return -1;
}

static int FreqSearch(vector<string>& keys, long start, long end, const string& ss){
    long left = start;
    long right = end - 1;

    int mid = 0;
    //定义域为[left,right]
    while(left <= right) {
        //此时标记left位置，防止下标越界
        mid = left + (right - left) / 2;
        if(keys[mid] == ss) {
            return mid;
        }
        if(keys[mid] > ss) {
            left = mid + 1;
        }
        if(keys[mid] < ss) {
            right = mid - 1;
        }
    }
    return -1;
}

class ConfigParameter {
public:
    int TRACE_NO;
    string STAT_FILE;
    string STREAM_FILE_PREFIX;
    string PATH_PREFIX;
    int HOTEST_FREQ_LIMIT;
    int FREQ_LIST_SIZE;
    int CMSKETCH_DEVIATION;
    int DAY;
    int DAY_NUM;
    int WINDOW_SIZE;
    int FREQ_LIMIT;
    int ONCE_READ_LIMIT;
    int EC_K;
    int EC_N;
    int THREAD_NUM;
    int LATENCY_NUM;
    int LOW_LIMIT;
    int CHUNK_SIZE;
    int SERVER_NUM;
    int variation;
    vector<pair<string, int>> SERVER_INFO;

    /*ConfigParameter() {
        this->STAT_FILE = "";
        this->STREAM_FILE_PREFIX= "";
        this->PATH_PREFIX = "";
        this->FREQ_LIST_SIZE = 0;
        this->DAY_NUM = 0;
        this->WINDOW_SIZE = 0;
        this->HOTEST_FREQ_LIMIT = 0;
        this->FREQ_LIMIT = 0;
        this->ONCE_READ_LIMIT = 0;
        this->CMSKETCH_DEVIATION = 0;
    }*/

    explicit ConfigParameter(enum workload_type wt = twitter, int trace_no = 2, int hotlim = 3444,  int snum = 9) {
        Json::Reader reader;
        Json::Value root;

// #ifdef LOCAL_TEST
//         ifstream in("../config_loc.json", ios::binary);
//         cout << "Local configure" << endl;
// #else
        ifstream in("../config.json", ios::binary);
        cout << "Standard configure" << endl;
// #endif

        if (!in.is_open()) {
            cout << "Error opening config file" << endl;
            return;
        }

        cout << "workloadtype = " << wt << endl;
        this->variation = snum;
        if(reader.parse(in, root)) {
            switch (wt) {
                case twitter:
                    // this->TRACE_NO = root["twitter"]["trace_no"].asInt();
                    this->TRACE_NO = trace_no;
                    this->STAT_FILE = "stat" + fmt::format("{:02d}", this->TRACE_NO);
                    this->STREAM_FILE_PREFIX= "workload"+fmt::format("{:02d}_", this->TRACE_NO);
                    this->PATH_PREFIX = root["twitter"]["path_prefix"].asString();
                    this->FREQ_LIST_SIZE = root["twitter"]["freq_list_size"].asInt();
                    this->DAY = root["twitter"]["day"].asInt();
                    //this->DAY = snum;
                    this->DAY_NUM = root["twitter"]["day_num"].asInt();
                    this->WINDOW_SIZE = root["twitter"]["window_size"].asInt();
                    // this->WINDOW_SIZE = snum;
                    //this->HOTEST_FREQ_LIMIT = root["twitter"]["hotest_freq_limit"].asInt();
                    this->HOTEST_FREQ_LIMIT = hotlim;
                    this->FREQ_LIMIT = root["twitter"]["freq_limit"].asInt();
                    //this->FREQ_LIMIT = snum;
                    this->ONCE_READ_LIMIT = root["twitter"]["once_read_num"].asInt();
                    this->CMSKETCH_DEVIATION = root["twitter"]["cmsketch_deviation"].asInt();
                    this->LATENCY_NUM = root["twitter"]["latency_num"].asInt();
                    this->LOW_LIMIT = root["twitter"]["low_limit"].asInt();
                    // this->SERVER_NUM = root["twitter"]["server_num"].asInt();
                    this->SERVER_NUM = this->variation;
                    break;
                case meta:
                    // this->TRACE_NO = root["meta"]["trace_no"].asInt();
                    this->TRACE_NO = trace_no;
                    this->STAT_FILE = "stat" + fmt::format("{:02d}", this->TRACE_NO);
                    this->STREAM_FILE_PREFIX= "kvcache_traces_";
                    this->PATH_PREFIX = root["meta"]["path_prefix"].asString() + to_string(this->TRACE_NO);
                    this->FREQ_LIST_SIZE = root["meta"]["freq_list_size"].asInt();
                    this->DAY = root["meta"]["day"].asInt();
                    //this->DAY = snum;
                    this->DAY_NUM = root["meta"]["day_num"].asInt();
                    // this->WINDOW_SIZE = root["meta"]["window_size"].asInt();
                    this->WINDOW_SIZE = snum;
                    //this->HOTEST_FREQ_LIMIT = root["meta"]["hotest_freq_limit"].asInt();
                    this->HOTEST_FREQ_LIMIT = hotlim;
                    this->FREQ_LIMIT = root["meta"]["freq_limit"].asInt();
                    //this->FREQ_LIMIT = snum;
                    this->ONCE_READ_LIMIT = root["meta"]["once_read_num"].asInt();
                    this->CMSKETCH_DEVIATION = root["meta"]["cmsketch_deviation"].asInt();
                    this->LATENCY_NUM = root["meta"]["latency_num"].asInt();
                    this->LOW_LIMIT = root["meta"]["low_limit"].asInt();
                    this->SERVER_NUM = root["meta"]["server_num"].asInt();
                    //this->SERVER_NUM = snum;
                    break;
                case ibm:
                    this->STAT_FILE = root["ibm"]["stat_file"].asString();
                    this->STREAM_FILE_PREFIX= root["ibm"]["stream_file_prefix"].asString();
                    this->PATH_PREFIX = root["ibm"]["path_prefix"].asString();
                    this->FREQ_LIST_SIZE = root["ibm"]["freq_list_size"].asInt();
                    //this->DAY = root["ibm"]["day"].asInt();
                    this->DAY = snum;
                    this->DAY_NUM = root["ibm"]["day_num"].asInt();
                    this->WINDOW_SIZE = root["ibm"]["window_size"].asInt();
                    this->HOTEST_FREQ_LIMIT = root["ibm"]["hotest_freq_limit"].asInt();
                    this->FREQ_LIMIT = root["ibm"]["freq_limit"].asInt();
                    this->ONCE_READ_LIMIT = root["ibm"]["once_read_num"].asInt();
                    this->CMSKETCH_DEVIATION = root["ibm"]["cmsketch_deviation"].asInt();
                    this->LATENCY_NUM = root["ibm"]["latency_num"].asInt();
                    this->LOW_LIMIT = root["ibm"]["low_limit"].asInt();
                    this->SERVER_NUM = root["ibm"]["server_num"].asInt();
                    //this->SERVER_NUM = snum;
                    break;
                default:
                    cout << "workloadtype2 = " << wt << endl;
            }
            this->EC_N = root["ec_n"].asInt();
            this->EC_K = root["ec_k"].asInt();
            this->THREAD_NUM = root["thread_num"].asInt();
            this->CHUNK_SIZE = root["chunk_size"].asInt();
            
            int count = 0;
            const Json::Value arrayObj = root["server_info"];
            for (const auto & i : arrayObj)
            {
                pair<string, int> tmp;
                tmp.first = i["ip"].asString();
                tmp.second = i["port"].asInt();
                SERVER_INFO.push_back(tmp);
                count ++;
                if(count == this->SERVER_NUM) break;
            }

            cout << "Configuration Parameters :" << this->PATH_PREFIX << endl;
        } else {
            cout << "parse error" << endl;
        }

        in.close();
    }

    ConfigParameter(const ConfigParameter& cp) {
        this->TRACE_NO = cp.TRACE_NO;
        this->STAT_FILE = cp.STAT_FILE;
        this->STREAM_FILE_PREFIX= cp.STREAM_FILE_PREFIX;
        this->PATH_PREFIX = cp.PATH_PREFIX;
        this->FREQ_LIST_SIZE = cp.FREQ_LIST_SIZE;
        this->DAY = cp.DAY;
        this->DAY_NUM = cp.DAY_NUM;
        this->WINDOW_SIZE = cp.WINDOW_SIZE;
        this->HOTEST_FREQ_LIMIT = cp.HOTEST_FREQ_LIMIT;
        this->FREQ_LIMIT = cp.FREQ_LIMIT;
        this->ONCE_READ_LIMIT = cp.ONCE_READ_LIMIT;
        this->CMSKETCH_DEVIATION = cp.CMSKETCH_DEVIATION;
        this->EC_K = cp.EC_K;
        this->EC_N = cp.EC_N;
        this->THREAD_NUM = cp.THREAD_NUM;
        this->SERVER_INFO = cp.SERVER_INFO;
        this->LATENCY_NUM = cp.LATENCY_NUM;
        this->LOW_LIMIT = cp.LOW_LIMIT;
        this->CHUNK_SIZE = cp.CHUNK_SIZE;
        this->SERVER_NUM = cp.SERVER_NUM;
    }

};

#endif //CORANA_CONFIG_H
