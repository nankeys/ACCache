//
// Created by Alfred on 2022/11/9.
//

#include <algorithm>
#include "SPCache.h"
#include "toolbox.h"
#include "MemcachedClient.h"

namespace SPCache {
    static pthread_mutex_t printmutex;
    workload_type wtype;
    double SPFactor = 0.000006;
    ConfigParameter cp;
    vector<key_param> ukeys;
    map<string, vector<string>> kmeta;

    void initial(workload_type wt, const int& snum) {
        wtype = wt;
        if(wtype == twitter) {
            cp = ConfigParameter(twitter, snum);
        } else {
            cp = ConfigParameter(ibm, snum);
        }

        ukeys = readStat(cp.PATH_PREFIX + "/" + cp.STAT_FILE);
    }

    void distribution() {
        vector<int> kvector;
        MemcachedClient mc(cp.SERVER_INFO);
       //size_t maxk = 0;

        for(auto &k: ukeys) {
            //cout << "key = " << k.key << ", size = " << k.size << ", k = " << k.freq * k.size * SPFactor << endl;
            //maxk = maxk > k.freq * k.size * SPFactor ? maxk: k.freq * k.size * SPFactor;
            int knum = max(int(min(size_t(k.freq * k.size * SPFactor), cp.SERVER_INFO.size())), 1);
            if(k.size / knum < 64) knum = k.size / 64;
            knum = max(knum, 1);
            //cout << "knum = " << knum << endl;
            kvector.push_back(knum);
        }

        //cout << "max k = " << maxk << endl;

        for(int i = 0; i < ukeys.size(); i ++) {
            for(int j = 0; j < kvector[i]; j++) {
                string ktmp = makeRandStr(100, true);
                kmeta[ukeys[i].key].push_back(ktmp);
                if(ukeys[i].size == 0) ukeys[i].size = 1;
                string value = string(ukeys[i].size / kvector[i], '1');
                mc.insert(ktmp.c_str(), value.c_str());
            }
        }

        cout << "Distribution finished!" << endl;
    }

    static void *twitter_query_exec(void *param) {
        timeit tt;
        MemcachedClient mc(cp.SERVER_INFO);

        string prefix = cp.PATH_PREFIX;

        pthread_mutex_lock(&printmutex);
        cout << ((thread_param *)param)->tid <<": twitter_query_exec" << endl;
        pthread_mutex_unlock(&printmutex);

        //pthread_mutex_lock(&printmutex);
        char filename[255];
        pthread_mutex_lock(&printmutex);
        //sprintf(filename, "d0t%dp%04d", cp.THREAD_NUM, ((thread_param *)param)->tid);
        snprintf(filename, sizeof(filename), "d%dt%dp%04d", cp.DAY, cp.THREAD_NUM, ((thread_param *)param)->tid);
        pthread_mutex_unlock(&printmutex);
        //sprintf(filename, "d0t128p%04d", ((thread_param *)param)->tid);
        string fname = prefix + "/" + filename;
        //pthread_mutex_unlock(&printmutex);

        //pthread_mutex_lock (&printmutex);
        cout << ((thread_param *)param)->tid <<",filename = " << fname << endl;
        //pthread_mutex_unlock (&printmutex);

        //pthread_mutex_lock (&printmutex);
        ifstream fin(fname);


        if(!fin) {
            cout <<  ((thread_param *)param)->tid <<": Error open trace file" << endl;
            exit(-1);
        }
        //pthread_mutex_unlock (&printmutex);

        pthread_mutex_lock (&printmutex);
        fprintf(stderr, "start benching using thread%u\n", ((thread_param *)param)->tid);
        pthread_mutex_unlock (&printmutex);


        vector<string> qkeys;
        while(fin.peek() != EOF) {

            char line[1000];
            long time_val;
            char query_key[200];
            int linenum;

            pthread_mutex_lock (&printmutex);
            linenum = 0;
            while(fin.peek() != EOF and linenum != cp.ONCE_READ_LIMIT) {
                fin.getline(line, 1000);
                time_val = strtol(strtok(line, ","), NULL, 10); // time
                qkeys.emplace_back(string(strtok(NULL, ",")));   //key
                linenum ++;
            }
            pthread_mutex_unlock (&printmutex);


            for(int it = 0; it != linenum; it ++) {
                string rst;
                bool flag;
                double max_time = 0;
                size_t tsize = 0;

                //int tloc = FreqSearch(ukeys, 0, ukeys.size(), qkeys[it]);
                if (kmeta.count(qkeys[it]) == 1) {
                    for(auto &pr: kmeta[qkeys[it]]) {
                        tt.start();
                        while (true) {
                            flag = mc.get(pr.c_str(), rst);
                            if (!rst.empty() || flag) break;
                        }
                        tsize += rst.size();
                        tt.end();
                        max_time =  max_time > tt.passedtime()? max_time: tt.passedtime();
                    }
                } else {
                    tt.start();
                    for(int ii = 0; ii < 3; ii ++) {
                        flag = mc.get(qkeys[it].c_str(), rst);
                        if (!rst.empty() || flag) break;
                    }
                    tsize = rst.size();
                    tt.end();
                    max_time = tt.passedtime();
                }


                //tail latency
                ((thread_param *) param)->latency.push(max_time);

                if (((thread_param *) param)->latency.size() >= cp.LATENCY_NUM) {
                    ((thread_param *) param)->latency.pop();
                }
                //total running time
                ((thread_param *) param)->runtime += max_time; //tt.passedtime();
                //sum ops
                ((thread_param *) param)->ops++;
                //sum size
                ((thread_param *) param)->size += tsize;
            }
            qkeys.clear();
            vector<string>().swap(qkeys);
        }
        fin.close();

        ((thread_param *)param)->thput_of_ops = ((thread_param *)param)->ops / ((thread_param *)param)->runtime;
        ((thread_param *)param)->thput_of_size = 1.0 * ((thread_param *)param)->size / ((thread_param *)param)->runtime / 1024;

        cout << "Total time: " << ((thread_param *)param)->runtime << endl
             << "Total ops: " << ((thread_param *)param)->ops << endl
             << "Total ops throughput: " << ((thread_param *)param)->thput_of_ops << endl
             << "Total sizes: " << ((thread_param *)param)->size << endl
             << "Total size throughput: " << ((thread_param *)param)->thput_of_size << " KB" << endl;


        //free(line);
        //memcached_server_list_free(server);
        pthread_exit(NULL);
    }

    static void *ibm_query_exec(void *param) {}

    void test(const int& snum) {
        cp = ConfigParameter(twitter, snum);
        pthread_t threads[cp.THREAD_NUM];
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);;

        pthread_mutex_init(&printmutex, NULL);

        thread_param tp[cp.THREAD_NUM];
        for (uint32_t t = 0; t < cp.THREAD_NUM; t++)
        {
            cout << "Threads = " << t << endl;
            //tp[t].queries = queries;
            tp[t].tid = t;
            // tp[t].sop     = sop_tmp;
            tp[t].ops = tp[t].size = 0;
            tp[t].runtime = tp[t].thput_of_ops = tp[t].thput_of_size = 0.0;
            int rci;
            if(wtype == ibm) {
                rci = pthread_create(&threads[t], &attr, ibm_query_exec, (void *) &tp[t]);
            } else {
                rci = pthread_create(&threads[t], &attr, twitter_query_exec, (void *) &tp[t]);
            }
            if (rci)
            {
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

        int nthreads = cp.THREAD_NUM;
        cout << "333333333333333333333" << endl;

        for (uint32_t t = 0; t < cp.THREAD_NUM; t++) {
            void *status;
            int rci = pthread_join(threads[t], &status);
            if (rci) {
                perror("error, pthread_join\n");
                exit(-1);
            }


            total_time = total_time > tp[t].runtime ? total_time: tp[t].runtime;
            total_ops += tp[t].ops;
            total_ops_thputs += tp[t].thput_of_ops;
            total_size += tp[t].size;
            total_size_thputs += tp[t].thput_of_size;
            while(!tp[t].latency.empty()) {
                latency.push_back(tp[t].latency.top());
                tp[t].latency.pop();
            }
        }
        cout << "4444444444444444444444444" << endl;
        sort(latency.rbegin(),latency.rend());

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

        ofstream fout("/data/result", ios::out|ios::app);
        //fout << "SP-Cache" << endl;
        //fout << snum << endl;
        fout << "SPCache" << "\t" << nthreads << "\t" << total_time << "\t" <<  total_ops << "\t" << total_ops_thputs << "\t"
             << total_size << "\t" << total_size_thputs << "\t"
             << latency95 << "\t" << latency99 << "\t" << latency9999 << endl;
        fout.close();

        pthread_attr_destroy(&attr);
        //return 0;
        ukeys.clear();
        vector<key_param>().swap(ukeys);
        kmeta.clear();
    }

}