//
// Created by Alfred on 2022/7/26.
//

#include "CMSketch.h"
#include "toolbox.h"
#include <cmath>
#include <vector>
#include <random>
#include <algorithm>
#include <queue>
#include <fstream>
#include <iostream>

using namespace std;

CMSketch::CMSketch(const long long& n, const int& deviation, const double& confidence) {
    this->n = n;
    long long number = n * n;
    cout << "deviation = " << deviation << endl;
    this->epsilon = (long double)  deviation / number;
    cout << "epsilon = " << epsilon << endl;
    this->m = size_t(ceil(M_E / this->epsilon));
    this->delta = confidence;
    this->k = int(ceil(log(1.0/confidence)));

    cout << "CMSketch k =" << this->k << endl;
    cout << "CMSketch m =" << this->m << endl;

    this->a = new int[this->k];
    this->b = new int[this->k];
    this->c = new int[this->k];

    this->bucket = vector<vector<int> >( this->k);
    for(int i = 0; i < this->k; i ++) {
        this->bucket[i] = vector<int>(this->m);
        this->bucket[i].resize(this->m);
    }

    static default_random_engine  e{random_device{}()};
    static uniform_int_distribution<int> u;

    for(int i = 0; i < this->k; i ++) {
        this->a[i] = u(e, decltype(u)::param_type(0, INT32_MAX));
        this->b[i] = u(e, decltype(u)::param_type(0, INT32_MAX));
        this->c[i] = u(e, decltype(u)::param_type(0, INT32_MAX));
    }
}

CMSketch::CMSketch(CMSketch &cms) {
    this->epsilon = cms.epsilon;
    this->m = cms.m;
    this->delta = cms.delta;
    this->k = cms.k;

    this->a = new int[this->k];
    this->b = new int[this->k];
    this->c = new int[this->k];

    this->bucket = vector<vector<int> >( this->k);
    for(int i = 0; i < this->k; i ++) {
        this->bucket[i] = vector<int>(this->m);
        this->bucket[i].resize(this->m);
    }

    for(int i = 0; i < this->k; i ++) {
        this->a[i] = cms.a[i];
        this->b[i] = cms.b[i];
        this->c[i] = cms.c[i];
    }
}

int CMSketch::hash(const int& a, const int& b, int& i) {
    unsigned int first, second;
    if(a < b) {
        first = b;
        second = a;
    } else {
        first = a;
        second = b;
    }
    int value = (this->a[i] * first + this->b[i] * second) % INT32_MAX + this->c[i];
    return value % this->m;
}

void CMSketch::add(const int& a, const int& b) {
    int loc;
    for(int i = 0; i < this->k; i ++) {
        loc = this->hash(a, b, i);
        this->bucket[i][loc] ++;
    }
}

int CMSketch::estimateFrequency(const int& a, const int& b) {
    int minimum = INT32_MAX;
    int loc;
    for(int i = 0; i < this->k; i ++) {
        loc = this->hash(a, b, i);
        minimum = min(minimum, this->bucket[i][loc]);
    }
    return minimum;
}

int CMSketch::find(const int& a, const int& b) {
    return estimateFrequency(a, b);
}

CMSketch::~CMSketch() {
    for(int i = 0; i < this->k; i ++) {
        this->bucket[i].clear();
    }
    //delete [] this->a;
    //delete [] this->b;
    //delete [] this->c;
}

void CMSketch::formalized() {
    for(int i = 0; i < this->k; i ++) {
        int tmp = *min_element(bucket[i].begin(), bucket[i].end());
        for(int j = 0; j < this->m; j ++) {
            bucket[i][j] = (bucket[i][j] - tmp) * (bucket[i][j] - tmp);
        }
    }
}

bool CMSketch::cutGraph() {
    vector<vector<int>> correlation_group;
    bool visited[n];
    bool flag = false;
    timeit tm;


    int max_connect = 0;
    int max_item = INT32_MAX;
    //vector<int> cgroup;

    //BFS
    int head;
    int connect_count;
    for(int i = 0; i < n; i ++) {
        visited[i] = false;
    }
    //sq.push(max_item);
    //visited[max_item] = true;

    size_t all_connection = 0;
    /*tm.start();

    for(int i = 0; i < n; i ++) {
        for(int j = i + 1; j < n; j ++) {
            all_connection += this->find(i, j);
        }
    }
    tm.end();
    cout << "Sum using " << tm.passedtime() << endl;*/
    all_connection = 62332412114572;
    cout << "Sum connection = " << all_connection << endl;

    cout << "Graph cutting initialized." << endl;

    while(true) {
        int count = 0;

        for(int i = 0; i < n; i ++) {
            //flag = flag && visited[i];
            if(!visited[i]) flag = true;
        }

        if(!flag) break;

        vector<int> cgroup;
        queue<int> sq;
        max_item = INT32_MAX;
        max_connect = 0;
        // get the maximum frequent initial node
        tm.start();
        for(int i = 0; i < n; i ++) {
            if(visited[i]) continue;
            int num = 0;
            for(int j = 0; j < n; j ++) {
                if(i == j || visited[j]) continue;
                //if(i > j) {
                num += this->find(i, j);
                //} else {
                //    num += this->find(j, i);
                //}
            }
            if (num > max_connect) {
                tm.end();
                max_connect = num;
                max_item = i;
                cout << "Now max_item = " << i << endl;
                cout << "Now max_connection = " << max_connect << endl;
                cout << "Now time elapse = " << tm.passedtime() << endl;
                count ++;
                //if(count == 10) break;
            }
        }
        cgroup.push_back(max_item);

        sq.push(max_item);
        visited[max_item] = true;

        cout << "initial node is " << max_item << ", frequency = " << max_connect << endl;
        head = sq.front();
        sq.pop();
        count = 0;
        for(int loci = 0; loci < n; loci ++) {
            if(count == 0) tm.start();
            if(!visited[loci] && this->find(head, loci) != 0 ) {
                vector<int> tmp = cgroup;
                //tmp.push_back(loci);
                CMSketch::group_insert(&tmp, loci);
                connect_count = this->getConnection(tmp, n, visited);
                if(connect_count <= max_connect) {
                    cout << "loci = " << loci << endl;
                    max_connect = connect_count;
                    //int cinc = this->getInnerConnection(tmp);
                    vector<int> comp = *CMSketch::complement(tmp, n);
                    //int cing = this->getInnerConnection(comp);
                    size_t cing = all_connection - connect_count - this->getInnerConnection(tmp);
                    //if(cinc < max_connect || cing < max_connect || comp.size() <= 2) {
                    if( cing < max_connect || comp.size() <= 2) {
                        //continue;
                        for(auto &pr : comp){
                            CMSketch::group_insert(&cgroup, pr);
                            visited[pr] = true;
                        }
                    }
                    CMSketch::group_insert(&cgroup, loci);

                    visited[loci] = true;
                    sq.push(loci);
                    auto *single_node = this->get_single_node(comp, visited);
                    if(!single_node->empty()) {
                        for(int & value : *single_node) {
                            CMSketch::group_insert(&cgroup, value);
                            visited[value] = true;
                        }
                        all_connection = all_connection - this->getInnerConnection(cgroup) - connect_count;
                    } else {
                        all_connection = cing;
                    }
                }
            }
            count ++;
            if(count == 0 || count % 1000 == 0) {
                tm.end();
                cout << "Time elapse : " << tm.passedtime() << endl;
            }
        }
        cout << "Group is " << endl;
        for(auto &pr : cgroup) {
            cout << pr << "\t";
        }
        cout << endl;
        correlation_group.push_back(cgroup);
        flag = false;
    }
    cout << "Sum groups = " << correlation_group.size() << endl;

    return false;
}

/*
 * Private
 */

int CMSketch::getConnection(const vector<int> &group, const int& n, bool *visited) {
    int num = 0;
    for(int value: group) {
        for(int i = 0; i < n; i ++) {
            if(visited[i]) continue;
            bool it = ::binary_search(group.begin(), group.end(), i);
            if(it) continue;
            num += this->find(value, i);
        }
    }
    return num;
}

int CMSketch::getInnerConnection(const vector<int> &group) {
    int num = 0;
    for(int pri : group) {
        for(int prj : group) {
            if(prj == pri) continue;
            num += this->estimateFrequency(pri, prj);
        }
    }
    return num;
}

// sorted: selective insertion
void CMSketch::group_insert(std::vector<int> *group, const int& value) {
    for(auto pr = group->begin(); pr != group->end(); pr ++) {
        if(*pr >= value) {
            group->emplace(pr, value);
            return;
        }
    }
    group->emplace(group->end(), value);
}

std::vector<int>* CMSketch::complement(std::vector<int> group, const int& n) {
    auto* rst = new vector<int>();
    auto pr = group.begin();
    for(int i = 0; i < n; i ++) {
        if(pr != group.end() && i != *pr) {
            rst->push_back(i);
        }else if(pr != group.end()) {
            pr ++;
        } else {
            rst->push_back(i);
        }
    }
    return rst;
}

vector<int>* CMSketch::get_single_node(std::vector<int> group, const bool *visited) {
    auto* rst = new vector<int>();
    for(auto pr = group.begin(); pr != group.end(); pr ++) {
        int num = 0;
        if(visited[*pr]) continue;
        for(auto prj = group.begin(); prj != group.end(); prj ++) {
            if(pr == prj) continue;
            if(visited[*prj]) continue;
            num += this->estimateFrequency(*pr, *prj);
        }
        if(num == 0) rst->push_back(*pr);
    }
    return rst;
}

void CMSketch::write2File(const std::string& filename) {

    ofstream fout;
    fout.open(filename,std::ofstream::out);

    if(!fout.is_open()) {
        cout << "Error opening files!" << endl;
        exit(-1);
    }

    fout << this->k << "\t" << this->m << endl;

    for(int i = 0; i < this->k; i ++) {
        fout << a[i] << "\t"
            << b[i] << "\t"
            << c[i] << endl;
    }

    for(auto &pri : bucket) {
        for(int & prj : pri) {
            fout << prj << " ";
        }
        fout << endl;
    }
    fout.close();
}

void CMSketch::load(const string &filename) {
    ifstream fin(filename);
    if(!fin.is_open()) {
        cout << "Error opening files!" << endl;
        exit(-1);
    }

    int tmp_k;
    size_t tmp_m;
    int tmp_a, tmp_b, tmp_c;
    fin >> tmp_k >> tmp_m;

    if(tmp_k != this->k || tmp_m != this->m) {
        cout << "Parameter Error" << endl;
        exit(-1);
    }

    for(int i = 0; i < this->k; i ++) {
        fin >> tmp_a >> tmp_b >> tmp_c;
        a[i] = tmp_a;
        b[i] = tmp_b;
        c[i] = tmp_c;
    }

    int tmp;
    for(auto & pri : bucket) {
        for(int & prj : pri) {
            fin >> tmp;
            prj = tmp;
        }
    }
    fin.close();

    cout << "CMSketch load complete." << endl;
}