//
// Created by Alfred on 2022/9/11.
//

#include "eccache.h"
#include "MemcachedClient.h"
#include <pthread.h>
#include "toolbox.h"
#include <algorithm>
#include <unistd.h>

namespace eccache {
    typedef struct {
        string pkey;
        string value;
    } pinfo;


    int k;
    int m;
    //const int LOW_LIMIT = 1024 * 1024;  // 1mb, lower than 1mb using replicas
    map<string, vector<string>> key_record; //<origin key, keys(replicas or ec)>
    vector<key_param> keys;
    workload_type wtype;
    ConfigParameter cpl;
    //static std::vector<std::pair<std::string, int>>& server_info;
    vector<agg_key> stripe_key;
    vector<vector<int> > chunk_keys;
    pthread_mutex_t printmutex;
    vector<pinfo> parity;


    map<string, vector<string>> keyRecord(vector<key_param> &keys) {
        map<string, vector<string>> key_record;
        for (auto &key: keys) {
            if (key.size > eccache::cpl.LOW_LIMIT) {
                int nstrip;
                for (nstrip = 1;; nstrip++) {
                    if (key.size / eccache::cpl.EC_K / nstrip <= eccache::cpl.LOW_LIMIT) break;
                }
                for (int strip = 0; strip < nstrip; strip++) {
                    for (int i = 0; i < eccache::cpl.EC_K; i++) {
                        char buffer[250];
                        snprintf(buffer, 250, "%s%04d%d", key.key.c_str(), strip, i);
                        key_record[key.key].push_back(buffer);
                    }
                    for (int i = 0; i < eccache::cpl.EC_N - eccache::cpl.EC_K; i++) {
                        char buffer[250];
                        snprintf(buffer, 250, "%s%04dp%d", key.key.c_str(), strip, i);
                        key_record[key.key].push_back(buffer);
                    }
                }
            } else {
                key_record[key.key].push_back(key.key);
                // cout << key.key << endl;
            }
        }
        return key_record;
    }

    void init(const ConfigParameter &cp, const workload_type &wt)
    {
        k = cp.EC_K;
        m = cp.EC_N - cp.EC_K;
        wtype = wt;
        cpl = cp;
        keys = readStat(cp.PATH_PREFIX + "/" + cp.STAT_FILE);
        key_record = keyRecord(eccache::keys);
        //readStat(cp.PATH_PREFIX + "/" + cp.STAT_FILE, skeys);
        sort(keys.begin(), keys.end(), key_string_comp);

        cout << "initialization finished" << endl;
    }

    void ibm_distribution() {
        MemcachedClient mc(cpl.SERVER_INFO);
        prealloc_encode pEncode(cpl.EC_N, cpl.EC_K);
        int n = k + m;
        ErasureCode ec(n, k);

        int tloc = FreqSearch(keys, 0, keys.size(), "00000971a34ea8a0");
        cout << "loc = " << tloc << endl;
        //key_param key = keys[tloc];
        cout << "last one = " << (keys.end() - 2)->key << endl;
        cout << "keys size = " << keys.size() << endl;
        for (auto &key: keys) {

            if (key.size > cpl.LOW_LIMIT) {
                int fill_len;
                //string object = string(key.size, '1');
                int nstrip = key_record[key.key].size() / n;
                auto strip_len = size_t(key.size / k / nstrip);
                //cout << "stripe len = " << strip_len << endl;
                if (size_t(strip_len * k * nstrip) < key.size) {
                    strip_len += 1;
                }
                cout << "key = " << key.key << endl;
                //cout << strip_len * k * nstrip - key.size << endl;
                //cout << "stripe len = " << strip_len << endl;

                //cout << "size = " << key.size << endl;
                //cout << "nstripe = " << nstrip << endl;
                fill_len = strip_len * k * nstrip - key.size;
                //cout << "filling len = " << fill_len << endl;
                //object += string(fill_len, '0');
                for (int i = 0; i < nstrip; i++) {
                    int flen;
                    string stripe;
                    if (i < nstrip - 1) {
                        stripe = string(k * strip_len, '1');
                    } else {
                        stripe = string(k * strip_len - fill_len, '1') + string(fill_len, '0');
                    }
                    uint8_t **source = ec.string2array(stripe, &flen);
                    if (flen != 0) {
                        cout << "error long" << endl;
                    }
                    ec.encode_data(pEncode, source, strip_len);
                    for (int j = 0; j < n; j++) {
                        cout << "i = " << i << ", j = " << j << endl;
                        cout << "stripe len = " << strip_len << endl;
                        string chunk = ec.get_line(source, strip_len, j);
                        mc.insert(key_record[key.key][i * k + j].c_str(), chunk.c_str());
                    }
                    for (int k = 0; k < cpl.EC_N; k++) {
                        delete source[k];
                    }
                    delete source;
                }
            } else {
                string object = string(key.size, '1');
                cout << key_record[key.key][0] << endl;
                //cout << key.size << endl;
                mc.insert(key_record[key.key][0].c_str(), object.c_str());

            }
        }
    }

    void twitter_distribution() {
        MemcachedClient mc(cpl.SERVER_INFO);
        int n = k + m;
        ErasureCode ec(n, k);

        prealloc_encode pEncode(cpl.EC_N, cpl.EC_K);


        size_t pr = 0;
        vector<size_t> large_object;
        int stripe_id = 0;
        while (true) {

            string stripe;
            string ckey[n];

            for (short i = 0; i < n; i++) {
                ckey[i] = makeRandStr(100, true);
            }

            size_t clength = cpl.CHUNK_SIZE * k;

            for (; pr != keys.size(); pr++) {
                if (keys[pr].size < clength) {
                    agg_key tmp;
                    short kno = stripe.size() / cpl.CHUNK_SIZE;
                    tmp.ckey = ckey[kno];
                    tmp.offset = stripe.size() % cpl.CHUNK_SIZE;
                    if (keys[pr].size == 0) keys[pr].size = 1;
                    tmp.length = keys[pr].size;
                    tmp.chunk_id = kno;
                    tmp.stripe_id = stripe_id;
                    clength -= keys[pr].size;
                    stripe_key.push_back(tmp);
                    stripe += string(keys[pr].size, '1');
                } else if (keys[pr].size > k * cpl.CHUNK_SIZE) {
                    //cout << "key = " << keys[pr].key << ", size = " << keys[pr].size << endl;
                    agg_key tmp;
                    tmp.ckey = makeRandStr(100, true);
                    stripe_key.push_back(tmp);
                    continue;
                    //exit(-1);
                } else {
                    stripe += string(clength, '#');
                    clength = 0;
                    break;
                }
            }

            if (clength != 0) stripe += string(clength, '#');

            int fill = 0;
            uint8_t **source = ec.string2array(stripe, &fill);
            ec.encode_data(pEncode, source, cpl.CHUNK_SIZE);

            /*cout << "stripe len = " << stripe.length() << endl;
            for(int i = 0; i < n; i ++) {
                cout << source[i] << endl;
            }*/

            for (int i = 0; i < m; i++) {
                pinfo tmp;
                tmp.value = ec.get_line(source, cpl.CHUNK_SIZE, k + i);
                //tmp.value = source[k+i];
                tmp.pkey = ckey[k + i];
                //cout << "parity " << i << " value is " << tmp.value << endl;
                parity.push_back(tmp);
            }

            if (pr == keys.size())
                break;
            stripe_id++;

            for(int i = 0; i < n; i ++) {
                delete [] source[i];
            }
            delete [] source;
        }

        cout << "22222222222222222222222222" << endl;

        for (int i = 0; i < keys.size(); i++) {
            int size = keys[i].size;
            if (size == 0) {
                size = 1;
            }
            //cout << "key = " << keys[i].key << ", size = " << size << endl;
            mc.gset(stripe_key[i].ckey.c_str(), keys[i].key.c_str(), string(size, '1').c_str());
        }

        for (auto &p: parity) {
            mc.insert(p.pkey.c_str(), p.value.c_str());
        }
    }

    void distribution() {
        if (wtype == ibm) {
            ibm_distribution();
        } else {
            twitter_distribution();
        }

        cout << "Objects distribution finished." << endl;
    }

    static void *twitter_query_exec(void *param) {
        timeit tt;
        MemcachedClient mc(cpl.SERVER_INFO);

        string prefix = cpl.PATH_PREFIX;

        pthread_mutex_lock(&printmutex);
        cout << ((thread_param *) param)->tid << ": twitter_query_exec" << endl;
        pthread_mutex_unlock(&printmutex);

        //pthread_mutex_lock(&printmutex);
        char filename[255];
        snprintf(filename, sizeof(filename), "t%02dd%dt%dp%04d", cpl.TRACE_NO, cpl.DAY, cpl.THREAD_NUM, ((thread_param *)param)->tid);
        //sprintf(filename, "d0t128p%04d", ((thread_param *)param)->tid);
        string fname = prefix + "/" + filename;
        //pthread_mutex_unlock(&printmutex);

        //pthread_mutex_lock (&printmutex);
        cout << ((thread_param *) param)->tid << ",filename = " << fname << endl;
        //pthread_mutex_unlock (&printmutex);

        //pthread_mutex_lock (&printmutex);
        ifstream fin(fname);


        if (!fin) {
            cout << ((thread_param *) param)->tid << ": Error open trace file" << endl;
            exit(-1);
        }
        //pthread_mutex_unlock (&printmutex);

        pthread_mutex_lock(&printmutex);
        fprintf(stderr, "start benching using thread%u\n", ((thread_param *) param)->tid);
        pthread_mutex_unlock(&printmutex);


        vector<string> qkeys;
        while (fin.peek() != EOF) {

            char line[1000];
            long time_val;
            char query_key[200];
            int linenum;

            pthread_mutex_lock(&printmutex);
            linenum = 0;
            while (fin.peek() != EOF and linenum != cpl.ONCE_READ_LIMIT) {
                fin.getline(line, 1000);
                time_val = strtol(strtok(line, ","), NULL, 10); // time
                qkeys.emplace_back(string(strtok(NULL, ",")));   //key
                linenum++;
            }
            pthread_mutex_unlock(&printmutex);


            for (int it = 0; it != linenum; it++) {
                string rst;
                bool flag;

                tt.start();
                int tloc = FreqSearch(keys, 0, keys.size(), qkeys[it]);
                if(tloc != -1) {
                    string ckeyt = stripe_key[tloc].ckey;
                    for(int ii = 0; ii < 3; ii ++) {
                        flag = mc.gget(ckeyt.c_str(), qkeys[it].c_str(), rst);
                        if (!rst.empty() || flag) break;
                    }
                } else {
                    for(int ii = 0; ii < 3; ii ++) {
                        flag = mc.get(qkeys[it].c_str(), rst);
                        if (!rst.empty() || flag) break;
                    }
                }
                tt.end();

                //tail latency
                ((thread_param *) param)->latency.push(tt.passedtime());

                if (((thread_param *) param)->latency.size() >= cpl.LATENCY_NUM) {
                    ((thread_param *) param)->latency.pop();
                }
                //total running time
                ((thread_param *) param)->runtime += tt.passedtime();
                //sum ops
                ((thread_param *) param)->ops++;
                //sum size
                ((thread_param *) param)->size += rst.size();
            }
            qkeys.clear();
            vector<string>().swap(qkeys);
        }
        fin.close();

        ((thread_param *) param)->thput_of_ops = ((thread_param *) param)->ops / ((thread_param *) param)->runtime;
        ((thread_param *) param)->thput_of_size =
                1.0 * ((thread_param *) param)->size / ((thread_param *) param)->runtime / 1024;

        cout << "Total time: " << ((thread_param *) param)->runtime << endl
             << "Total ops: " << ((thread_param *) param)->ops << endl
             << "Total ops throughput: " << ((thread_param *) param)->thput_of_ops << endl
             << "Total sizes: " << ((thread_param *) param)->size << endl
             << "Total size throughput: " << ((thread_param *) param)->thput_of_size << " KB" << endl;


        //free(line);
        //memcached_server_list_free(server);
        pthread_exit(NULL);
    }

    static void *meta_query_exec(void *param) {
        timeit tt;
        MemcachedClient mc(cpl.SERVER_INFO);

        string prefix = cpl.PATH_PREFIX;

        pthread_mutex_lock(&printmutex);
        cout << ((thread_param *) param)->tid << ": meta_query_exec" << endl;
        pthread_mutex_unlock(&printmutex);

        //pthread_mutex_lock(&printmutex);
        char filename[255];
        snprintf(filename, sizeof(filename), "t%02dd%dt%dp%04d", cpl.TRACE_NO, cpl.DAY, cpl.THREAD_NUM, ((thread_param *)param)->tid);
        //sprintf(filename, "d0t128p%04d", ((thread_param *)param)->tid);
        string fname = prefix + "/" + filename;
        //pthread_mutex_unlock(&printmutex);

        //pthread_mutex_lock (&printmutex);
        cout << ((thread_param *) param)->tid << ",filename = " << fname << endl;
        //pthread_mutex_unlock (&printmutex);

        //pthread_mutex_lock (&printmutex);
        ifstream fin(fname);


        if (!fin) {
            cout << ((thread_param *) param)->tid << ": Error open trace file" << endl;
            exit(-1);
        }
        //pthread_mutex_unlock (&printmutex);

        pthread_mutex_lock(&printmutex);
        fprintf(stderr, "start benching using thread%u\n", ((thread_param *) param)->tid);
        pthread_mutex_unlock(&printmutex);


        vector<string> qkeys;
        while (fin.peek() != EOF) {

            char line[1000];
            size_t time_val;
            char query_key[200];
            int linenum;

            pthread_mutex_lock(&printmutex);
            linenum = 0;
            while (fin.peek() != EOF and linenum != cpl.ONCE_READ_LIMIT) {
                fin.getline(line, 1000);
                if(cpl.TRACE_NO == 202206) {
                    // time_val = strtol(strtok(line, ","), NULL, 10); // time
                    qkeys.emplace_back(string(strtok(line, ",")));   //key
                } else if(cpl.TRACE_NO == 202401) {
                    time_val = strtol(strtok(line, ","), NULL, 10); // time
                    qkeys.emplace_back(string(strtok(NULL, ",")));   //key
                }
                linenum++;
            }
            pthread_mutex_unlock(&printmutex);


            for (int it = 0; it != linenum; it++) {
                string rst;
                bool flag;

                tt.start();
                int tloc = FreqSearch(keys, 0, keys.size(), qkeys[it]);
                if(tloc != -1) {
                    string ckeyt = stripe_key[tloc].ckey;
                    for(int ii = 0; ii < 3; ii ++) {
                        flag = mc.gget(ckeyt.c_str(), qkeys[it].c_str(), rst);
                        if (!rst.empty() || flag) break;
                    }
                } else {
                    for(int ii = 0; ii < 3; ii ++) {
                        flag = mc.get(qkeys[it].c_str(), rst);
                        if (!rst.empty() || flag) break;
                    }
                }
                tt.end();

                //tail latency
                ((thread_param *) param)->latency.push(tt.passedtime());

                if (((thread_param *) param)->latency.size() >= cpl.LATENCY_NUM) {
                    ((thread_param *) param)->latency.pop();
                }
                //total running time
                ((thread_param *) param)->runtime += tt.passedtime();
                //sum ops
                ((thread_param *) param)->ops++;
                //sum size
                ((thread_param *) param)->size += rst.size();
            }
            qkeys.clear();
            vector<string>().swap(qkeys);
        }
        fin.close();

        ((thread_param *) param)->thput_of_ops = ((thread_param *) param)->ops / ((thread_param *) param)->runtime;
        ((thread_param *) param)->thput_of_size =
                1.0 * ((thread_param *) param)->size / ((thread_param *) param)->runtime / 1024;

        cout << "Total time: " << ((thread_param *) param)->runtime << endl
             << "Total ops: " << ((thread_param *) param)->ops << endl
             << "Total ops throughput: " << ((thread_param *) param)->thput_of_ops << endl
             << "Total sizes: " << ((thread_param *) param)->size << endl
             << "Total size throughput: " << ((thread_param *) param)->thput_of_size << " KB" << endl;


        //free(line);
        //memcached_server_list_free(server);
        pthread_exit(NULL);
    }

    static void *ibm_query_exec(void *param) {
        cout << ((thread_param *) param)->tid << " random_query_exec: 11111111111111111" << endl;
        MemcachedClient mc(cpl.SERVER_INFO);
        timeit tt;


        char filename[255];
        snprintf(filename, sizeof(filename), "d%dt%dp%04d", cpl.DAY, cpl.THREAD_NUM, ((thread_param *)param)->tid);
        string fname = filename;
        //string fname = "/home/flnan/ibm/t16_ibm00d01p" + to_string(((thread_param *)param)->tid);
        pthread_mutex_lock(&printmutex);
        cout << ((thread_param *) param)->tid << ",filename = " << fname << endl;
        pthread_mutex_unlock(&printmutex);
        ifstream fin(fname);


        if (!fin) {
            cout << "Error open trace file" << endl;
            exit(-1);
        }

        pthread_mutex_lock(&printmutex);
        cout << ((thread_param *) param)->tid << " random_query_exec:2222222222222222222222222" << endl;
        pthread_mutex_unlock(&printmutex);

        pthread_mutex_lock(&printmutex);
        fprintf(stderr, "start benching using thread%u\n", ((thread_param *) param)->tid);
        pthread_mutex_unlock(&printmutex);


        while (fin.peek() != EOF) {
            char line[1000];
            long time_val;
            char query_key[100];
            size_t value_len;
            int group;
            bool flag;

            string qkeys[cpl.ONCE_READ_LIMIT];
            int linenum;

            pthread_mutex_lock(&printmutex);
            linenum = 0;
            while (fin.peek() != EOF and linenum != cpl.ONCE_READ_LIMIT) {
                fin.getline(line, 1000);
                time_val = strtol(strtok(line, " "), NULL, 10); // time
                strcpy(query_key, strtok(NULL, " "));
                qkeys[linenum] = string(strtok(NULL, " "));   //key
                linenum++;
            }
            pthread_mutex_unlock(&printmutex);

            //cout << "Search  = " << FreqSearch(keys, 0 , keys.size(), "56f24fa744aa5b54");

            for (int it = 0; it != linenum; it++) {
                string rst;
                tt.start();
                if (qkeys[it][qkeys[it].size() - 1] == '\n') qkeys[it] = qkeys[it].substr(0, qkeys[it].size() - 1);
                if (qkeys[it][qkeys[it].size() - 1] == '\r') qkeys[it] = qkeys[it].substr(0, qkeys[it].size() - 1);
                //strcpy(query_key, qkeys[it].c_str());
                int loc = FreqSearch(keys, 0, keys.size(), qkeys[it]);
                if (loc == -1) {
                    //cout << (qkeys[it][qkeys[it].size() - 1] == '\n') << endl;
                    //cout << "Error keys = " << qkeys[it] << endl;
                    //exit(-1);
                    for(int ii = 0; ii < 3; ii ++) {//while (true) {
                        flag = mc.get(qkeys[it].c_str(), rst);
                        if (!rst.empty()) break;
                    }
                } else {
                    if (keys[loc].size < cpl.LOW_LIMIT) {
                        flag = mc.get(key_record[qkeys[it]][0].c_str(), rst);
                        //cout << rst << endl;
                        //sleep(100);
                    } else {
                        int stripe_num = key_record[qkeys[it]].size() / cpl.EC_N;
                        for (int i = 0; i < stripe_num; i++) {
                            for (int j = 0; j < cpl.EC_K + 1; j++) {
                                flag = mc.get(key_record[qkeys[it]][i].c_str(), rst);
                                //sleep(100);
                            }
                        }
                    }
                }
                tt.end();
                double tmp_time = tt.passedtime();

                //tail latency
                /*auto pr = ((thread_param *) param)->latency.begin();
                for (; pr != ((thread_param *) param)->latency.end(); pr++) {
                    if (tmp_time >= *pr) {
                        break;
                    }
                }
                ((thread_param *) param)->latency.emplace(pr, tmp_time);*/
                ((thread_param *) param)->latency.push(tmp_time);
                if (((thread_param *) param)->latency.size() >= cpl.LATENCY_NUM) {
                    ((thread_param *) param)->latency.pop();
                }
                //total running time
                ((thread_param *) param)->runtime += tmp_time;
                //sum ops
                ((thread_param *) param)->ops++;
                //sum size
                ((thread_param *) param)->size += value_len;
            }
        }
        fin.close();

        ((thread_param *) param)->thput_of_ops = ((thread_param *) param)->ops / ((thread_param *) param)->runtime;
        ((thread_param *) param)->thput_of_size =
                1.0 * ((thread_param *) param)->size / ((thread_param *) param)->runtime / 1024;

        cout << "Total time: " << ((thread_param *) param)->runtime << endl
             << "Total ops: " << ((thread_param *) param)->ops << endl
             << "Total ops throughput: " << ((thread_param *) param)->thput_of_ops << endl
             << "Total sizes: " << ((thread_param *) param)->size << endl
             << "Total size throughput: " << ((thread_param *) param)->thput_of_size << " KB" << endl;

        pthread_exit(NULL);
    }

    void test(const ConfigParameter& cp, const int& snum) {
        cpl = cp;
        pthread_t threads[cpl.THREAD_NUM];
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);;

        pthread_mutex_init(&printmutex, NULL);

        thread_param tp[cpl.THREAD_NUM];
        for (uint32_t t = 0; t < cpl.THREAD_NUM; t++) {
            cout << "Threads = " << t << endl;
            //tp[t].queries = queries;
            tp[t].tid = t;
            // tp[t].sop     = sop_tmp;
            tp[t].ops = tp[t].size = 0;
            tp[t].runtime = tp[t].thput_of_ops = tp[t].thput_of_size = 0.0;
            int rci;
            if (wtype == ibm) {
                rci = pthread_create(&threads[t], &attr, ibm_query_exec, (void *) &tp[t]);
            } else if(wtype == meta) {
                rci = pthread_create(&threads[t], &attr, meta_query_exec, (void *) &tp[t]);
            } else if(wtype == twitter) {
                rci = pthread_create(&threads[t], &attr, twitter_query_exec, (void *) &tp[t]);
            }
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

        int nthreads = cpl.THREAD_NUM;
        cout << "333333333333333333333" << endl;

        for (uint32_t t = 0; t < cpl.THREAD_NUM; t++) {
            void *status;
            int rci = pthread_join(threads[t], &status);
            if (rci) {
                perror("error, pthread_join\n");
                exit(-1);
            }


            total_time = total_time > tp[t].runtime ? total_time : tp[t].runtime;
            total_ops += tp[t].ops;
            total_ops_thputs += tp[t].thput_of_ops;
            total_size += tp[t].size;
            total_size_thputs += tp[t].thput_of_size;
            while (!tp[t].latency.empty()) {
                latency.push_back(tp[t].latency.top());
                tp[t].latency.pop();
            }
        }
        cout << "4444444444444444444444444" << endl;
        sort(latency.rbegin(), latency.rend());

        double latency95 = latency[total_ops - int(total_ops * 0.95)];
        double latency99 = latency[total_ops - int(total_ops * 0.99)];
        double latency9999 = latency[total_ops - int(total_ops * 0.9999)];

        cout << "Total time: " << total_time << endl
             << "Total ops: " << total_ops << endl
             << "Total op throughput: " << total_ops_thputs << endl
             << "Total sizes: " << total_size << endl
             << "Total size throughput: " << total_size_thputs << endl
             << "95\% latency: " << latency95 << endl
             << "99\% latency: " << latency99 << endl
             << "99.99\% latency: " << latency9999 << endl;

        ofstream fout(cpl.PATH_PREFIX + "/result", ios::out|ios::app);
        //fout << snum << endl;
        fout << "ECCache" << "\t" << nthreads << "\t" << total_time << "\t" << total_ops << "\t" << total_ops_thputs << "\t"
             << total_size << "\t" << total_size_thputs << "\t"
             << latency95 << "\t" << latency99 << "\t" << latency9999 << endl;
        fout.close();

        pthread_attr_destroy(&attr);
        //return 0;
    }

}