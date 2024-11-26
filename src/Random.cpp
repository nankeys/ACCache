//
// Created by Alfred on 2022/10/19.
//

#include <algorithm>
#include "Random.h"
#include "MemcachedClient.h"
#include "toolbox.h"
#include "pthread.h"

ConfigParameter cp;
vector<key_param> keys;
workload_type wt;
pthread_mutex_t printmutex;
int server_num = 0;

void random_read_file(const workload_type& type, const int& snum) {
    wt =type;
    cp = ConfigParameter(type, snum);
    keys = readStat(cp.PATH_PREFIX + "/" + cp.STAT_FILE);
}

void twitter_init(){
    char single = '1';
    MemcachedClient mc(cp.SERVER_INFO);
    cout << "init: 1111111111111111" << endl;
    for(auto & key : keys) {
        int tmp_size = key.size == 0? 1:key.size;
        string value = string(tmp_size, single);
        mc.insert(key.key.c_str(), value.c_str());
    }
    cout << "init: 22222222222222222" << endl;


    cout << "Twitter init finished." << endl;
}

void random_init() {
    if(wt == twitter) {
        twitter_init();
    }

    cout << "Workload init finished." << endl;
}

void *twitter_query_exec(void *param) {
    timeit tt;
    MemcachedClient mc(cp.SERVER_INFO);

    string prefix = cp.PATH_PREFIX;

    pthread_mutex_lock(&printmutex);
    cout << ((thread_param *)param)->tid <<": twitter_query_exec" << endl;
    pthread_mutex_unlock(&printmutex);

    //pthread_mutex_lock(&printmutex);
    char filename[255];
    //sprintf(filename, "d0t%dp%04d", cp.THREAD_NUM, ((thread_param *)param)->tid);
    snprintf(filename, sizeof(filename), "d%dt%dp%04d", cp.DAY, cp.THREAD_NUM, ((thread_param *)param)->tid);
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
    vector<string> ops;
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
            time_val = strtol(strtok(NULL, ","), NULL, 10);   //key len
            time_val = strtol(strtok(NULL, ","), NULL, 10);   //value len
            time_val = strtol(strtok(NULL, ","), NULL, 10);   //client id
            ops.emplace_back(string(strtok(NULL, ",")));   //ops
            linenum ++;
        }
        pthread_mutex_unlock (&printmutex);


        for(int it = 0; it != linenum; it ++) {
            string rst;
            bool flag;

            tt.start();
//            if (ops[it] == "set") {
//                flag = mc.insert(qkeys[it].c_str(), "1111");
//            } else {
                for(int ii = 0; ii < 3; ii ++) {//while (true) {
                    flag = mc.get(qkeys[it].c_str(), rst);
                    if (!rst.empty()) break;
                }
            //}
            tt.end();

            //tail latency
            /*int left = 0;
            int right = ((thread_param *) param)->latency.size() - 1;
            int mid = 0;
            //找a[i]应该插入的位置
            while (left <= right) {
                mid = (left + right) / 2;
                if (tt.passedtime() < ((thread_param *) param)->latency[mid]) {
                    left = mid + 1;
                } else {
                    right = mid + -1;
                }
            }
            ((thread_param *) param)->latency.emplace(((thread_param *) param)->latency.begin()+left, tt.passedtime());*/
            ((thread_param *) param)->latency.push(tt.passedtime());
            /*auto pr = ((thread_param *) param)->latency.begin();
            for (; pr != ((thread_param *) param)->latency.end(); pr++) {
                if (tt.passedtime() >= *pr) {
                    break;
                }
            }
            ((thread_param *) param)->latency.emplace(pr, tt.passedtime());*/
            if (((thread_param *) param)->latency.size() >= cp.LATENCY_NUM) {
               // ((thread_param *) param)->latency.pop_back();
               // ((thread_param *) param)->latency.shrink_to_fit();
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

void *ibm_query_exec(void *param) {
    return nullptr;
}

void random_test(const workload_type& type, const int& snum) {
    wt = type;
    cp = ConfigParameter(type, snum);

    //cout << "11111111111111111" << endl;
    //random_read_file();
    //cout << "22222222222222222" << endl;
    //random_init();
    //cout << "33333333333333333" << endl;

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
        if(wt == twitter) {
            rci = pthread_create(&threads[t], &attr, twitter_query_exec, (void *) &tp[t]);
        } else {
            rci = pthread_create(&threads[t], &attr, ibm_query_exec, (void *) &tp[t]);
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

    int nthreads = cp.THREAD_NUM;
    cout << "end:333333333333333333333" << endl;

    for (uint32_t t = 0; t < cp.THREAD_NUM; t++) {
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

    ofstream fout("/data/result", ios::out|ios::app);
    //fout << snum << endl;
    fout << "Random" << "\t" << nthreads << "\t" << total_time << "\t" <<  total_ops << "\t" << total_ops_thputs << "\t"
         << total_size << "\t" << total_size_thputs << "\t"
         << latency95 << "\t" << latency99 << "\t" << latency9999 << endl;
    fout.close();

    pthread_attr_destroy(&attr);
}


