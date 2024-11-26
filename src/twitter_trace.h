//
// Created by Alfred on 2022/7/22.
//

#ifndef CORANA_TWITTER_TRACE_H
#define CORANA_TWITTER_TRACE_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include <jsoncpp/json/json.h>

#include "config.h"
#include "FreqList.h"
#include "FreqTable.h"
#include "ListNode.h"
#include "toolbox.h"
#include "CMSketch.h"

// global definitions
vector<string>* freqkeys;

class tkeys
{
public:
    tkeys(const string& a, const int b)
            : key(a), freq(b)
    {}
    string key;
    int freq;
    bool operator<(const tkeys& m) const {
        return freq > m.freq;
    }
};

vector<string>* getFeqKeys(string str, int size);
vector<string>* readStream(string fname, int lnum, long long& cpos);
int isFreqkeys(string str);

void twitter_trace()
{
    ConfigParameter cp;
    FreqList flist(cp.FREQ_LIST_SIZE, cp.FREQ_LIMIT);
    //FreqTable ftable(cp.HOTEST_NUM);
    CMSketch ftable(9e10, 900);
    vector<string>* rstream;    // the stream list

    // temp variables
    string cnts;
    int stream_size;
    int flag, readnum;
    int width; // the width of the window
    long long cur_pos = 0;  // the current position of the trace file
    long long ftail; // the tail position of the trace file
    string tname;   // the name of the reading trace file
    vector<string>* tmp;

    //temp for timing
    //auto start = system_clock::now();
    //auto end = system_clock::now();
    //auto duration = duration_cast<microseconds>(end - start);
    timeit t = timeit();

    for(int day = 0; day < cp.DAY_NUM; day ++)
    {
        cout << "The " << day + 1 << "th day." << endl;

        flag = 0;
        readnum = 0;
        int tmp_size = cp.ONCE_READ_LIMIT;
        /**
         * Read the trace and get the hotest keys
         */
        ifstream fin(cp.STAT_FILE);

        if (!fin.is_open()) {
            cout << "Error opening file: " << cp.STAT_FILE << endl;
            exit(1);
        }
        for(int i = 0; i < day; i ++) {
            getline(fin, cnts);
            getline(fin, cnts);
            getline(fin, cnts);
        }
        getline(fin, cnts);
        cout << "day = " << cnts << endl;
        getline(fin, cnts);
        getline(fin, cnts);
        fin.close();

        // Get the top-k most frequent keys
        freqkeys = getFeqKeys(cnts, cp.HOTEST_NUM);
        ofstream oof("Freqkeys" + to_string(day));
        for(auto & freqkey : *freqkeys) {
            oof << freqkey << endl;
        }
        oof.close();
        cout << "finished reading hotkyes" << endl;

        // first read the stream

        tname = cp.PATH_PREFIX + cp.STREAM_FILE_PREFIX + to_string(day);
        // get the tail position of the file
        fin.open(tname);
        fin.seekg(0, ios::end);
        ftail = fin.tellg();
        cout << "Tail position: " << ftail << endl;
        fin.close();

        cur_pos = 0;
        rstream = readStream(tname, cp.ONCE_READ_LIMIT, cur_pos);
        stream_size = cp.ONCE_READ_LIMIT;

        /*
         * Begin the Correlation Analysis
         */
        cout << "Correlation Analysis start" << endl;
        while(stream_size != 0) {
            //stream_size = rstream->size();
            if (flag == 0) {
                //start = system_clock::now();
                t.start();
            }
            if (stream_size < cp.WINDOW_SIZE + 100 && tmp_size >= cp.ONCE_READ_LIMIT) {
                tmp = readStream(tname, cp.ONCE_READ_LIMIT, cur_pos);
                rstream->insert(rstream->end(), tmp->begin(), tmp->end());
                tmp_size = tmp->size();
                stream_size += tmp_size; //tmp->size();
                delete tmp;
                readnum++;
            }

            int i = 0;

            auto stream_start = rstream->begin();

            int loc1 = isFreqkeys(*stream_start);
            while( loc1 == -1) {
                stream_start ++;
                loc1 = isFreqkeys(*stream_start);
            }

            int size = rstream->end() - stream_start;
            width = size > cp.WINDOW_SIZE? cp.WINDOW_SIZE: size;
            for(auto iter = stream_start + 1; iter < stream_start + width; iter ++, i ++) {
                int loc2 = isFreqkeys(*iter);
                if( loc2 == -1 || loc1 == loc2) {
                    continue;
                }

                ListNode ln = ListNode(loc1, loc2, 0);
                int rst = ftable.find(ln.first, ln.second);
                if(rst != 0) {
                    ftable.add(ln.first, ln.second);
                } else {
                    auto index = flist.insert(ln);
                    //auto index = flist.index(ln);
                    if(flist.isHot(index)) {
                        ftable.add(index->first, index->second);
                        flist.del(index);
                    }
                }

            }
            rstream->erase(rstream->begin(), stream_start);
            stream_size -= stream_start - rstream->begin();
            flag ++;
            if(flag % 1000000 == 0) {
                //end = system_clock::now();
                //duration = duration_cast<microseconds>(end - start);
                t.end();

                cout << "Time used: "<< t.passedtime() << endl;
                //	if(flag / 1000000 > 66)
//                    break;
            }
        }

        delete rstream;
        delete freqkeys;
        cout << "Current point position = " << cur_pos << endl;
        ofstream fout("ftable_rst" + to_string(day));
        for(int i = 0; i < cp.HOTEST_NUM; i ++) {
            for(int j = i + 1; j < cp.HOTEST_NUM; j ++)
                fout << to_string(i) + "," + to_string(j) << "," << ftable.find(i, j) << endl;
        }
        fout.close();
        flist.clear();
        //ftable.clear();
        t.end();
        cout << "Day " << day <<" Total time used: "<< t.passedtime() << "s" << endl;
    }

}

vector<string>* getFeqKeys(const string& str, const int size)
{
    vector<string>* dkeys = new vector<string>();
    vector<tkeys> twkeys;
    int i;

    Json::Reader reader;
    Json::Value value;
    Json::Value::Members mem;

    if (reader.parse(str, value)) {
        if(value.size() < (unsigned)size) {
            return nullptr;
        }
        mem = value.getMemberNames();
        for(auto iter = mem.begin(); iter != mem.end(); iter ++) {
            tkeys tmp(*iter, value[*iter].asInt());
            twkeys.push_back(tmp);
        }
    }
    partial_sort(twkeys.begin(), twkeys.begin()+size, twkeys.end());
    i = 0;
    for(auto iter = twkeys.begin(); iter != twkeys.end() && i < size; i ++, iter ++) {
        dkeys->push_back(iter->key);
    }
    sort(dkeys->begin(), dkeys->end());
    return dkeys;
}

vector<string>* readStream(string fname, int lnum, long long& cpos)
{
    ifstream in(fname);
    in.seekg(cpos, ios::beg);
    auto* vec = new vector<string>();
    string lcnt, rst;
    vector<string> tmp;
    int i;

    if (!in.is_open()) {
        cout << "Error opening file: " << fname << endl;
        exit(0);
        //return nullptr;
    }

    i = 0;
    while(getline(in, lcnt) && i < lnum) {
        tmp = split(lcnt, ',');
        rst =  tmp[1];
        /*auto pr = equal_range(freqkeys->begin(), freqkeys->end(), rst);
        if((pr.first)->compare(rst) == 0) {
            vec->push_back(to_string(pr.first - freqkeys->begin()));
            i ++;
        }*/
        vec->push_back(rst);
        i ++;
    }

    cpos = in.tellg();
    in.close();
    return vec;
}

int isFreqkeys(string str)
{
    auto pr = equal_range(freqkeys->begin(), freqkeys->end(), str);
    if((pr.first)->compare(str) == 0) {
        return pr.first - freqkeys->begin();
    }
    return -1;
}

#endif //CORANA_TWITTER_TRACE_H
