//
// Created by Alfred on 2022/9/14.
//

#include "OurScheme.h"
#include "FreqList.h"
#include "CMSketch.h"
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include <unordered_set>
#include <random>
#include <queue>
#include "fmt/core.h"
#include <python3.10/Python.h>
#include "MemcachedClient.h"

#include "toolbox.h"

using namespace std;

vector<string> gkeys;
pthread_mutex_t oprintmutex;
ConfigParameter cpOur;
vector<unordered_set<string>> fgroup;
vector<key_param> okeys;
vector<key_param>::iterator freq_limit_pos;

vector<vector<int>> distribute_group(int bin_num) {
    vector<vector<int>> group_distribution;
    Py_Initialize();
    if (!Py_IsInitialized())
    {
        printf("Initialization Failed");
        exit(-1);
    }

    PyRun_SimpleString("import sys");
    // string path = "sys.path.append('"+ cpOur.PATH_PREFIX + "/')";
    string path = "sys.path.append('../')";

    cout << "Python file path = " << path << endl;
    PyRun_SimpleString(path.c_str());


    PyObject * pModule = NULL;
    PyObject * pFunc = NULL;
    pModule = PyImport_ImportModule("stats");
    if (pModule==NULL)
    {
        cout << "Not found" << endl;
    }
    pFunc = PyObject_GetAttrString(pModule, "group_distribution2");
    PyObject* pDict = PyDict_New();
    for(int i = freq_limit_pos - okeys.begin(); i < okeys.size(); i ++) {
        PyDict_SetItemString(pDict, to_string(i).c_str(), Py_BuildValue("i", okeys[i].freq));
    }
    PyObject* pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, pDict);
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", bin_num));

    PyObject* pRet = PyObject_CallObject(pFunc, pArgs);

    if (PyList_Check(pRet)) {
        // okay, it's a list
        for (Py_ssize_t i = 0; i < PyList_Size(pRet); ++i) {
            vector<int> tmp;
            PyObject* next = PyList_GetItem(pRet, i);
            if(PyList_Check(next)) {
                for (Py_ssize_t j = 0; j < PyList_Size(next); ++j) {
                    PyObject* snext = PyList_GetItem(next, j);
                    if(!PyUnicode_Check(snext)) {
                        cout << "Wrong return" << endl;
                        exit(-1);
                    }

                    tmp.push_back(atoi((char*)PyUnicode_DATA(snext)));
                }
                group_distribution.push_back(tmp);
            }

        }
    }

    //cout << "Finish grouping" << endl;
    return group_distribution;
}

void OurScheme::getFreqKeys(const string &stats_file) {
    ifstream fin(stats_file);

    if(!fin.is_open()) {
        cout << "Error opening stats file!" << endl;
        exit(-1);
    }

    while(fin.peek() != EOF) {
        key_param tmp;
        fin >> tmp.key >> tmp.size >> tmp.freq;
        okeys.push_back(tmp);
    }

    okeys.pop_back();

    sort(okeys.begin(), okeys.end(), key_freq_comp);

    freq_limit_pos = okeys.end();
    for(auto pr = okeys.begin(); pr != okeys.end(); pr ++) {
        if (pr->freq == cpOur.HOTEST_FREQ_LIMIT || ((pr + 1) != okeys.end() && pr->freq > cpOur.HOTEST_FREQ_LIMIT && (pr + 1)->freq < cpOur.HOTEST_FREQ_LIMIT))
            freq_limit_pos = pr;
        total_freq += pr->freq;
    }


    sort(okeys.begin(), freq_limit_pos, key_string_comp);
    sort(freq_limit_pos, okeys.end(), key_string_comp);

    cout << "NUmber of hot keys = " << freq_limit_pos - okeys.begin() << endl;

    ofstream fout(fmt::format("{}/freqkeys{:02d}_{}", cpOur.PATH_PREFIX, cpOur.TRACE_NO, cpOur.WINDOW_SIZE), ios::out);
    if(!fout.is_open()) {
        cout << "Error opening file to write freqkeys." << endl;
        exit(-1);
    }

    for(auto pr = okeys.begin(); pr != freq_limit_pos + 1; pr++) {
        fout << pr->key << endl;
    }
    fout.close();

    cout << "finished reading hotkyes" << endl;
}

vector<string> *OurScheme::readStream(const string& fname, int lnum, long long int &cpos) const {
    ifstream in(fname);
    in.seekg(cpos, ios::beg);
    auto* vec = new vector<string>();
    string lcnt, rst;
    vector<string> tmp;
    int i;
    bool flag;

    if (!in.is_open()) {
        cout << "Error opening file: " << fname << endl;
        exit(0);
    }

    i = 0;
    while(getline(in, lcnt) && i < lnum) {
        switch (wtype) {
            case meta:
                tmp = split(lcnt, ',');
                if(tmp[0] == "key") {
                    flag = true; //202206
                    continue;
                } else if (tmp[0] == "op_time") {
                    flag = false; //202401
                    continue;
                }

                if(flag) {
                    rst =  tmp[0];
                } else {
                    rst = tmp[1];
                }
                break;

            case twitter:
                tmp = split(lcnt, ',');
                rst =  tmp[1];
                break;
            case ibm:
                tmp = split(lcnt, ' ');
                rst =  tmp[2];
                break;
        }
        vec->push_back(rst);
        i ++;
    }

    cpos = in.tellg();
    in.close();
    return vec;
}

int OurScheme::isFreqKeys(const string &key) {
    /*key_param tmp;
    tmp.key = key;

    auto pr = find(keys.begin(), freq_limit_pos + 1, tmp);
    if(pr != freq_limit_pos + 1) {
        return pr - keys.begin();
    }
    return -1;*/
    return FreqSearch(okeys, 0, freq_limit_pos - okeys.begin() + 1, key);
}

vector<int>* OurScheme::group_stat(const string& filename, const int& group_num)  {
    ifstream fin(filename);
    if(!fin.is_open()) {
        cout << "Error opening files!" << endl;
        exit(-1);
    }

    auto groups = new vector<int> [group_num+100];

    cout << "group read: group num = " << group_num << endl;


    size_t num, size, freq;
    int node;
    int it = 0;
    while(fin.peek() != EOF) {

        fin >> num >> size >> freq;
        if(fin.eof() or fin.fail() or fin.bad()) break;
        //cout << num << ", " << size << ", " << freq << endl;
        for(int i = 0; i < num; i ++) {
            fin >> node;
            groups[it].push_back(node);
        }
        it ++;
    }

    gstat = vector<int>(group_num);
    gsize = vector<int>(group_num);

    for(int i = 0; i < group_num; i ++) {
        int sum_freq = 0;
        int sum_size = 0;
        for(int & pr : groups[i]) {
            sum_freq += okeys[pr].freq;
            sum_size += okeys[pr].size;
        }
        //cout << sum_freq << endl;
        //cout << "origin Group " << i << " frequent = " << sum_freq << endl;
        gstat[i] = sum_freq;
        gsize[i] = sum_size;
        //cout << "origin Group " << i << " frequent = " << gstat[i] << endl;
    }

    gstat.resize(group_num);
    gsize.resize(group_num);

    return groups;
}

OurScheme::OurScheme(enum workload_type wt)
{
    string stat_file;

    this->wtype = wt;

    cpOur = ConfigParameter(wt);
    total_freq = 0;
    cout << "Hot objects frequent limit = " << cpOur.HOTEST_FREQ_LIMIT << endl;

    stat_file = cpOur.PATH_PREFIX + "/" + cpOur.STAT_FILE;

    cout << "stat file = " << stat_file << endl;

    getFreqKeys(stat_file);

    cout << "CMSketch = " << freq_limit_pos - okeys.begin() << endl;
    cout << "FreqSearch = " << FreqSearch(okeys, 0, freq_limit_pos-okeys.begin()+1, (--freq_limit_pos)->key) << endl;
    //this->ftable = CMSketch(freq_limit_pos - keys.begin(), cpOur.CMSKETCH_DEVIATION);
    this->ftable = FreqTable(freq_limit_pos - okeys.begin());

}

void OurScheme::CorrelationAnalysis() {
    FreqList flist(cpOur.FREQ_LIST_SIZE, cpOur.HOTEST_FREQ_LIMIT);
    //CMSketch ftable();

    long long current_pos = 0;
    long long tail_pos;
    int flag = 0;
    timeit t;

    string workload_file = cpOur.PATH_PREFIX+ "/" + cpOur.STREAM_FILE_PREFIX + to_string(cpOur.TRACE_NO) + "_" + to_string(cpOur.DAY);

    cout << "workload_file = " << workload_file << endl;
    // get the tail position of the file
    ifstream fin(workload_file);
    if (!fin.is_open()) {
        cout << "Error opening file: " << workload_file << endl;
        exit(0);
    }
    fin.seekg(0, ios::end);
    tail_pos = fin.tellg();
    cout << "Tail position: " << tail_pos << endl;
    fin.close();

    t.start();
    vector<string> *rstream = readStream(workload_file, cpOur.ONCE_READ_LIMIT, current_pos);
    t.end();
    cout << "Read the stream pos Time used: "<< t.passedtime() << endl;

    /*
    * Begin the Correlation Analysis
    */
    cout << "Correlation Analysis start" << endl;
    auto start_pos = rstream->begin();
    cout << *start_pos << endl;
    int start_index = 0;

    while(!rstream->empty()) {
        if((start_pos + 1) != rstream->end() && (rstream->size() - (start_pos-rstream->begin())) < cpOur.WINDOW_SIZE + 100 && current_pos != -1) {
            if(start_index == rstream->size()) {
                rstream->erase(rstream->begin(), rstream->end());
            } else {
                rstream->erase(rstream->begin(), start_pos + 1);
            }
            vector<string> *stmp = readStream(workload_file, cpOur.ONCE_READ_LIMIT, current_pos);
            rstream->insert(rstream->end(), stmp->begin(), stmp->end());
            stmp->clear();
            delete stmp;
            stmp = nullptr;
            start_pos = rstream->begin();
            start_index = 0;
        }

        int loc1 = isFreqKeys(*start_pos);

        //if() break;

        while(loc1 == -1 && (start_pos + 1) <= rstream->end()) {
            start_pos ++;
            start_index ++;
            loc1 = isFreqKeys(*start_pos);
        }

        if(loc1 == -1 && current_pos == -1){
            break;
        } else if(loc1 == -1) {
            continue;
        }

        if(start_pos >= rstream->end())
            break;

        //cout << rstream->size() << endl;
        //cout << start_index << endl;

        int size = rstream->end() - start_pos;
        int size1 = rstream->size() - start_index;

        int width = size > cpOur.WINDOW_SIZE? cpOur.WINDOW_SIZE: size;

        for(auto pr = start_pos + 1; pr != start_pos + width; pr ++) {
            int loc2 = isFreqKeys(*pr);
            if(loc2 == -1 || loc2 == loc1) continue;

            int rst = ftable.find(loc1, loc2);

            //if(rst != 0) {
                ftable.add(loc1, loc2);
                ftable.add(loc1, loc2);
            // } else {
            //     auto index = flist.insert(ListNode(loc1, loc2));
            //     if(flist.isHot(index)) {
            //         ftable.add(index->first, index->second);
            //         ftable.add(index->first, index->second);
            //         flist.del(index);
            //     }
            // }

        }

        start_pos++;
        start_index++;
        flag ++;
        if(flag % 100000000 == 0) {
            t.end();
            cout << "Time used: "<< t.passedtime() << endl;
            //	if(flag / 1000000 > 66)
//                    break;
        }
    }
    //delete rstream;
    cout << "Current point position = " << current_pos << endl;
    flist.clear();
    ftable.write4louvain(fmt::format("{}/louvain{}", cpOur.PATH_PREFIX, cpOur.TRACE_NO));
}

void OurScheme::test() {
    size_t num_of_keys = okeys.size();
    std::default_random_engine e;
    std::uniform_int_distribution<int> u(0, num_of_keys);

    //for (int i = 0; i < 100; ++i) {
        int loc = freq_limit_pos - okeys.begin(); //u(e);
        cout << "the origin " << loc << " th key = " << okeys[loc].key << endl;
        int nloc = FreqSearch(okeys, 0, freq_limit_pos - okeys.begin() + 1, okeys[loc].key);
        //if(nloc == -1) {
        //    nloc = FreqSearch(okeys, freq_limit_pos - okeys.begin(), okeys.size(), okeys[loc].key);
        //}
        if(nloc != -1) {
            cout << "the found " << nloc << " th key = " << okeys[nloc].key << endl;
        } else {
            cout << "Not found" << endl;
        }
    //}

}

void OurScheme::distribute(const string &group_file, const int& group_num, const int& node_num) {
    vector<set<int> > distributed_group;
    vector<int> distributed_freq;
    vector<int> distributed_size;


    auto group_divided = group_stat(group_file, group_num);
    cout << "group num = " << group_num << endl;
    cout << "group size = " << group_divided->size() << endl;

    int dinic_length = 2  + gstat.size() + node_num + 1;
    int common_items = gstat.size();

    vector<vector<pair<int, int>>> graph;

    graph.resize(dinic_length);
    for(int i = 0; i < dinic_length; i ++) {
        //graph[i].resize(dinic_length);
        for(int j = 0; j < dinic_length; j ++) {
            graph[i].push_back(pair(0,0));
        }
    }
    for(int i = 0; i < common_items + 2; i ++) {
        graph[1][i + 2].first = gstat[i];
        graph[i + 2][1].first = -gstat[i];
    }
    size_t gfreq = 0;
    for(int i = 0; i < common_items; i ++) {
        gfreq += gstat[i];
    }

    for(int i = 0; i < node_num; i ++) {
        for(int j = 0; j < gstat.size(); j ++) {
            graph[2 + gstat.size() + i][2 + j].first = 0 - int(gfreq * 1.0 / node_num);
            graph[2 + j][2 + gstat.size() + i].first = int(gfreq * 1.0 / node_num);
        }
        graph[2 + gstat.size() + i][dinic_length - 1].first = int(gfreq * 1.0 / node_num);
        graph[dinic_length - 1][2 + gstat.size() + i].first = 0 - int(gfreq * 1.0 / node_num);
    }

    /*for(int i = 1; i < dinic_length; i ++) {
        //graph[i].resize(dinic_length);
        for(int j = 1; j < dinic_length; j ++) {
            cout << graph[i][j].first << "," << graph[i][j].second << " ";
        }
        cout << endl;
    }*/

    int re = dinic(graph, dinic_length - 1);

    /*cout << "re = " << re << endl;
    for(int i = 1; i < dinic_length; i ++) {
        //graph[i].resize(dinic_length);
        for(int j = 1; j < dinic_length; j ++) {
            cout << graph[i][j].first << "," << graph[i][j].second << " ";
        }
        cout << endl;
    }*/

    vector<int> loc;
    loc.resize(gstat.size());
    for(int i = 2; i < 2 + gstat.size(); i ++) {
        int max = 0, max_index = 0;
        for(int j = 0; j < node_num; j ++) {
            if(graph[i][2 + gstat.size() + j].second > max) {
                max = graph[i][2 + gstat.size() + j].second;
                max_index = j;
            }
        }
        loc[i - 2] = max_index;
    }

    cout << endl;
    for(int i = 0; i < gstat.size(); i ++)
        cout << gstat[i] << "," << loc[i] << " ";
    cout << endl;

    distributed_group.resize(node_num);
    distributed_freq.resize(node_num);
    distributed_size.resize(node_num);

    for(int i = 0; i < node_num; i ++)
        distributed_freq[i] = 0;

    for(int i = 0; i < gstat.size(); i ++) {
        distributed_group[loc[i]].insert(group_divided[i].begin(), group_divided[i].end());
        distributed_freq[loc[i]] += gstat[i];
        distributed_freq[loc[i]] += gsize[i];
    }

    vector<vector<int>> cold_items = distribute_group(node_num);

    for(int i = 0; i < node_num; i ++) {
        distributed_group[i].insert(cold_items[i].begin(), cold_items[i].end());
    }

    MemcachedClient mc(cpOur.SERVER_INFO);

    gkeys = mc.get_server_key(100);
    cout << "Got MemcachedClient keys" << endl;

    for(int i = 0; i < node_num; i ++) {
        for(auto &pr: distributed_group[i]) {
            if(okeys[pr].size == 0) okeys[pr].size = 1;
            string value(okeys[pr].size, '1');
            mc.gset(gkeys[i].c_str(), okeys[pr].key.c_str(), value.c_str());
        }
    }
    
    //cout << "1111111111111111111111111111111" << endl;

    fgroup.resize(node_num);
    for(int i = 0; i < node_num; i ++) {
        for(auto &pr: distributed_group[i]) {
            fgroup[i].emplace(okeys[pr].key);
        }
    }

    /*for(auto pr = freq_limit_pos; pr != okeys.end(); pr ++) {
        size_t size = 0;
        if(pr->size == 0) size = 1;
        else size = pr->size;
        string value(size, '1');
        mc.insert(pr->key.c_str(), value.c_str());
    }*/



    cout << "Distributed Finished" << endl;
}

void *twitter_query_exec(void *param) {
    timeit tt;
    MemcachedClient mc(cpOur.SERVER_INFO);

    string prefix = cpOur.PATH_PREFIX;

    pthread_mutex_lock(&oprintmutex);
    //cout << ((thread_param *)param)->tid <<": twitter_query_exec" << endl;
    pthread_mutex_unlock(&oprintmutex);

    //pthread_mutex_lock(&oprintmutex);
    char filename[255];
    //sprintf(filename, "d0t%dp%04d", cpOur.THREAD_NUM, ((thread_param *)param)->tid);
    snprintf(filename, sizeof(filename), "w%02dd%dt%dp%04d", cpOur.TRACE_NO, cpOur.DAY, cpOur.THREAD_NUM, ((thread_param *)param)->tid);
    //sprintf(filename, "d0t128p%04d", ((thread_param *)param)->tid);
    string fname = prefix + "/" + filename;
    //pthread_mutex_unlock(&oprintmutex);

    //pthread_mutex_lock (&oprintmutex);
   // cout << ((thread_param *)param)->tid <<",filename = " << fname << endl;
    //pthread_mutex_unlock (&oprintmutex);

    //pthread_mutex_lock (&oprintmutex);
    ifstream fin(fname);


    if(!fin) {
        cout <<  ((thread_param *)param)->tid <<": Error open trace file" << endl;
        exit(-1);
    }
    //pthread_mutex_unlock (&oprintmutex);

    pthread_mutex_lock (&oprintmutex);
    //fprintf(stderr, "start benching using thread%u\n", ((thread_param *)param)->tid);
    pthread_mutex_unlock (&oprintmutex);

    //cout << "Location = " << FreqSearch(okeys, 0, freq_limit_pos - okeys.begin() + 1, "vi3.j3_S1b.Iz.9S.sC");
    //cout << "keys size = "  << okeys.size() << endl;


    vector<string> qkeys;
    while(fin.peek() != EOF) {

        if(fin.eof() or fin.fail() or fin.bad()) break;

        char line[1000];
        long time_val;
        char query_key[200];
        int linenum;

        pthread_mutex_lock (&oprintmutex);
        linenum = 0;
        while(fin.peek() != EOF and linenum != cpOur.ONCE_READ_LIMIT) {
            fin.getline(line, 1000);
            time_val = strtol(strtok(line, ","), NULL, 10); // time
            qkeys.emplace_back(string(strtok(NULL, ",")));   //key
            linenum ++;
        }
        pthread_mutex_unlock (&oprintmutex);



        for(int it = 0; it < linenum; it ++) {
            //vector<string> skeys;
            size_t key_num = 0;
            char **skey;
            bool flag;
            size_t rsize;
            int lg = -1;
            int sops = 1;
            size_t sbegin = -1;
            char sgkey[255];

            if (qkeys[it][qkeys[it].size() - 1] == '\n') qkeys[it] = qkeys[it].substr(0, qkeys[it].size() - 1);
            if (qkeys[it][qkeys[it].size() - 1] == '\r') qkeys[it] = qkeys[it].substr(0, qkeys[it].size() - 1);

            //tt.start();
            /*int loc = FreqSearch(okeys, 0, freq_limit_pos - okeys.begin() + 1, qkeys[it]);
            if(loc == -1) {
                loc = FreqSearch(okeys, freq_limit_pos - okeys.begin(), okeys.size(), qkeys[it]);
            }*/
            for (int i = 0; i < fgroup.size(); i++) {
                if(fgroup[i].count(qkeys[it]) == 1) {
                    lg = i;
                }
            }
            if(lg != -1) {
                strcpy(sgkey, gkeys[lg].c_str());
                //skeys.emplace_back(qkeys[it]);
                sbegin = it;
                key_num ++;
                for (++it;it < linenum; it++) {
                    /*int loc2 = FreqSearch(okeys, 0, freq_limit_pos - okeys.begin() + 1, qkeys[it]);
                    if(loc2 == -1) {
                        loc2 = FreqSearch(okeys, freq_limit_pos - okeys.begin(), okeys.size(), qkeys[it]);
                    }*/
                    if(fgroup[lg].count(qkeys[it]) == 1) {
                       // skeys.emplace_back(qkeys[it]);
                        key_num ++;
                        //if(key_num >= 10) break;
                    } else {
                        it --;
                        break;
                    }
                }
                if(key_num > 1) {
                    size_t key_len[key_num];
                    skey = new char* [key_num];
                    //key_len = new size_t [key_num];
                    for(int i = 0; i < key_num; i ++) {
                        skey[i] = new char[255];
                        strcpy(skey[i], qkeys[sbegin + i].c_str());
                        key_len[i] = strlen(skey[i]); //skeys[i].size();
                    }
                    rsize = 0;
                    //while(rsize == 0) {
                    //tt.start();
                    //for(int ii = 0; ii < 3; ii ++) {
                        //rsize = mc.mgget(sgkey, skey, key_len, key_num);
                        memcached_return rc;
                        memcached_result_st results_obj;
                        memcached_result_st *results;

                        results= memcached_result_create(mc.memc, &results_obj);

                        //size_t key_len[key_num];
                        size_t gkey_len = strlen(sgkey);
                        size_t value_len = 0;

                        //for(int i = 0 ; i < key_num; i ++) {
                        //    key_len[i] = strlen(key[i]);
                        //}
                        tt.start();
                        rc = memcached_mget_by_key(mc.memc, sgkey, gkey_len, skey, key_len, key_num);
                        tt.end();
                        //if(rc != MEMCACHED_SUCCESS)
                        //    return 0;

                        while ((results= memcached_fetch_result(mc.memc, &results_obj, &rc)))
                        {
                            //if(rc != MEMCACHED_SUCCESS) {
                            //value_len = 0;
                            //    break;
                            //}

                            value_len += memcached_result_length(results);
                        }

                        //cout << "value len = " << value_len << endl;

                        memcached_result_free(&results_obj);
                    //    if (value_len != 0) {
                            rsize = value_len;
                    //        break;
                    //    }
                    //}

                    sops =  key_num;

                    for(int i = 0; i < key_num; i ++) {
                        delete [] skey[i];
                    }
                    delete [] skey;
                    //delete [] key_len;
                } else {
                    string rst;
                    char thekey[500];
                    //strcpy(sgkey, gkeys[lg].c_str());
                    strcpy(thekey, qkeys[sbegin].c_str());
                    rsize = 0;

                    char * v;

                    for(int ii = 0; ii < 3; ii ++)
                    {
                        //flag = mc.gget(sgkey, thekey, rst);
                        uint32_t flags = 0;
                        memcached_return rc;
                        size_t value_length;

                        tt.start();
                        v = memcached_get_by_key(mc.memc, sgkey, strlen(sgkey), thekey, strlen(thekey), &value_length, &flags, &rc);
                        tt.end();

                        if (v != NULL) {
                            break;
                        }
                    }

                    if(v != NULL) {
                        rst = v;
                    } else {
                        rst = "";
                    }

                    free(v);

                    rsize = rst.size();
                    sops = 1;
                }

            } else {
                string rst;
                char thekey[500];
                strcpy(thekey, qkeys[it].c_str());
                rsize = 0;
                tt.start();
                for(int ii = 0; ii < 1; ii ++)
                {
                    flag = mc.get(thekey, rst);
                    if (!rst.empty()) break;
                }
                tt.end();

                rsize = rst.size();
                sops = 1;
            }
            //tt.end();

            for(int i = 0; i < sops; i ++) {
                ((thread_param *) param)->latency.push(tt.passedtime() / sops);
                //((thread_param *) param)->latency.push(tt.passedtime());
                if (((thread_param *) param)->latency.size() >= cpOur.LATENCY_NUM) {
                    ((thread_param *) param)->latency.pop(); 
                }
            }
            //total running time
            ((thread_param *) param)->runtime += tt.passedtime();
            //sum ops
            ((thread_param *) param)->ops += sops;
            //sum size
            ((thread_param *) param)->size += rsize;
        }
        qkeys.clear();
        vector<string>().swap(qkeys);
    }
    fin.close();

    ((thread_param *)param)->thput_of_ops = ((thread_param *)param)->ops / ((thread_param *)param)->runtime;
    ((thread_param *)param)->thput_of_size = 1.0 * ((thread_param *)param)->size / ((thread_param *)param)->runtime / 1024;

    // cout << "Total time: " << ((thread_param *)param)->runtime << endl
    //      << "Total ops: " << ((thread_param *)param)->ops << endl
    //      << "Total ops throughput: " << ((thread_param *)param)->thput_of_ops << endl
    //      << "Total sizes: " << ((thread_param *)param)->size << endl
    //      << "Total size throughput: " << ((thread_param *)param)->thput_of_size << " KB" << endl;


    //free(line);
    //memcached_server_list_free(server);
    pthread_exit(NULL);
}

void OurScheme::query() {
    //cpOur = ConfigParameter(twitter, snum);
    pthread_t threads[cpOur.THREAD_NUM];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);;

    pthread_mutex_init(&oprintmutex, NULL);
 
    thread_param tp[cpOur.THREAD_NUM];
    for (uint32_t t = 0; t < cpOur.THREAD_NUM; t++)
    //for (uint32_t t = 0; t < 1; t++)
    {
       // cout << "Threads = " << t << endl;
        //tp[t].queries = queries;
        tp[t].tid = t;
        // tp[t].sop     = sop_tmp;
        tp[t].ops = tp[t].size = 0;
        tp[t].runtime = tp[t].thput_of_ops = tp[t].thput_of_size = 0.0;
        int rci;
        if(wtype == twitter) {
            rci = pthread_create(&threads[t], &attr, twitter_query_exec, (void *) &tp[t]);
        } //else {
            //rci = pthread_create(&threads[t], &attr, ibm_query_exec, (void *) &tp[t]);
        //}
        if (rci) {
            perror("failed: pthread_create\n");
            exit(-1);
        }

    }

    double total_ops_thputs = 0.0;
    long double total_size_thputs = 0.0;
    int total_ops = 0;
    double total_time = 0.0;
    unsigned long long total_size = 0;
    vector<double> latency;

    int nthreads = cpOur.THREAD_NUM;
   // cout << "end:333333333333333333333" << endl;

    for (uint32_t t = 0; t < cpOur.THREAD_NUM; t++) {
    //for (uint32_t t = 0; t < 1; t++) {
        void *status;
        int rci = pthread_join(threads[t], &status);
        if (rci) {
            perror("error, pthread_join\n");
            exit(-1);
        }


        total_time = total_time > tp[t].runtime? total_time: tp[t].runtime;
        total_ops += tp[t].ops;
        total_ops_thputs += tp[t].thput_of_ops;
        total_size += tp[t].size;
        total_size_thputs += tp[t].thput_of_size;
        while(!tp[t].latency.empty()) {
            latency.push_back(tp[t].latency.top());
            tp[t].latency.pop();
        }
        //latency.insert(latency.end(), tp[t].latency.begin(),tp[t].latency.end());
    }
    sort(latency.rbegin(),latency.rend());
    double latency95 = latency[total_ops - int(total_ops * 0.95)];
    double latency99 = latency[total_ops - int(total_ops * 0.99)];
    double latency9999 = latency[total_ops - int(total_ops * 0.9999)];

    cout << "Total time: " << total_time << endl
         << "Total ops: " << total_ops << endl
         << "Total op throughput: " << total_ops_thputs << endl
         << "Total sizes: " << total_size << endl
         << "Total size throughput: " << total_size_thputs << endl
         << "95\% latency: " << latency95 *1000 << endl
         << "99\% latency: " << latency99 *1000 << endl
         << "99.99\% latency: " << latency9999 *1000 << endl;

    ofstream fout(cpOur.PATH_PREFIX + "/result", ios::out|ios::app);
    //fout << snum << endl;
    //fout << "Our Scheme" << endl;
    fout << "AC-Cache" << "\t" << nthreads << "\t" << total_time << "\t" <<  total_ops << "\t" << total_ops_thputs << "\t"
         << total_size << "\t" << total_size_thputs << "\t"
         << latency95 << "\t" << latency99 << "\t" << latency9999 << endl;
    fout.close();

    pthread_attr_destroy(&attr);
}

OurScheme::~OurScheme() {
    gkeys.clear();
    okeys.clear();
}
