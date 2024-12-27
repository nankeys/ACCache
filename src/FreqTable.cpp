//
// Created by Alfred on 2022/7/22.
//

#include <fstream>
#include <iostream>
#include <algorithm>
#include <queue>
#include "FreqTable.h"
#include "toolbox.h"

FreqTable::FreqTable():n(0) {
}

FreqTable::FreqTable(const size_t& length):n(length) {
    size_t len = n * (n - 1) / 2;
    cout << "len = " << len << endl;
    this->ftable = vector<int>(len);
    this->ftable.resize(len);
    this->ftable.shrink_to_fit();
}

FreqTable::FreqTable(const FreqTable &_a):n(_a.n) {
    size_t len = _a.n * (_a.n - 1) / 2;
    this->ftable = vector<int>(len);
    this->ftable.resize(len);
    this->ftable.shrink_to_fit();
}

int FreqTable::estimateFrequency(const int& a, const int& b) {
    size_t index = loc(a, b);
    return this->ftable[index];
}

int FreqTable::find(const int& a, const int& b) {
    return estimateFrequency(a, b);
}

size_t FreqTable::loc(const int& a, const int& b) const {
    int first, second;
    size_t index;

    if(a == b) return -1;
    else if(a > b) {
        first = b;
        second = a;
    } else {
        first = a;
        second = b;
    }
    index = (2 * n - first - 1) * first / 2 + (second - first - 1);
    return index;
}

int FreqTable::at(const int& index) {
    return this->ftable[index];
}

void FreqTable::add(const int &a, const int &b, const int &num) {
    size_t index = loc(a, b);

    for(int i = 0; i < num; i ++) {
        this->ftable[index] ++;
    }
}

void FreqTable::write2File(const string &filename) {
    ofstream fout(filename);

    if(!fout.is_open()) {
        cout << "Error opening files!" << endl;
        exit(-1);
    }

    fout << this->n << endl;

    for(auto &pr: ftable) {
        fout << pr << '\t';
    }
    fout << endl;

    fout.close();
}

void FreqTable::write4louvain(const std::string& filename) {
    try {
        BufferedWriter writer(filename, 1024);

        for(int i = 0; i != n; i ++) {
            for(int j = i + 1; j != n; j ++) {
                int res = find(i, j);
                if(res == 0) continue;
                writer.write(to_string(i) + "\t" + to_string(j) + "\t" + to_string(res) + "\n");
        }
    }

        // 析构时会自动 flush 数据
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void FreqTable::load(const string &filename) {
    ifstream fin(filename);
    if(!fin.is_open()) {
        cout << "Error opening files!" << endl;
        exit(-1);
    }

    int tmp_n;
    fin >> tmp_n;

    if(tmp_n != n) {
        cout << "Parameter Error" << endl;
        exit(-1);
    }

    size_t length = ftable.size();
    int tmp_value;
    for(size_t i = 0; i < length; i ++) {
        fin >> tmp_value;
        ftable[i] = tmp_value;
    }
}

bool FreqTable::cutGraph() {
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
    tm.start();

    for(int i = 0; i < n; i ++) {
        for(int j = i + 1; j < n; j ++) {
            all_connection += this->find(i, j);
        }
    }
    tm.end();
    cout << "Sum using " << tm.passedtime() << endl;
    //all_connection = 62332412114572;
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
                FreqTable::group_insert(&tmp, loci);
                connect_count = this->getConnection(tmp, n, visited);
                if(connect_count <= max_connect) {
                    cout << "loci = " << loci << endl;
                    max_connect = connect_count;
                    //int cinc = this->getInnerConnection(tmp);
                    vector<int> comp = *FreqTable::complement(tmp, n);
                    int cing = this->getInnerConnection(comp);
                    //size_t cing = all_connection - connect_count - this->getInnerConnection(tmp);
                    //if(cinc < max_connect || cing < max_connect || comp.size() <= 2) {
                    if( cing < max_connect || comp.size() <= 2) {
                        //continue;
                        for(auto &pr : comp){
                            FreqTable::group_insert(&cgroup, pr);
                            visited[pr] = true;
                        }
                    }
                    FreqTable::group_insert(&cgroup, loci);

                    visited[loci] = true;
                    sq.push(loci);
                    auto *single_node = this->get_single_node(comp, visited);
                    if(!single_node->empty()) {
                        for(int & value : *single_node) {
                            FreqTable::group_insert(&cgroup, value);
                            visited[value] = true;
                        }
                        //all_connection = all_connection - this->getInnerConnection(cgroup) - connect_count;
                    } //else {
                        //all_connection = cing;
                    //}
                }
            }
            count ++;
            if(count == 0 || count % 10000 == 0) {
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

    ofstream fout("/home/flnan/groups");

    if(!fout.is_open()) {
        cout << "Error opening files!" << endl;
        exit(-1);
    }

    fout << correlation_group.size() << endl;

    for(auto &pri : correlation_group) {
        for(auto &prj: pri) {
            fout << prj << "\t";
        }
        fout << endl;
    }
    fout.close();

    return false;
}

int FreqTable::getConnection(const vector<int> &group, const size_t& n, bool *visited) {
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

int FreqTable::getInnerConnection(const vector<int> &group) {
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
void FreqTable::group_insert(std::vector<int> *group, const size_t& value) {
    for(auto pr = group->begin(); pr != group->end(); pr ++) {
        if(*pr >= value) {
            group->emplace(pr, value);
            return;
        }
    }
    group->emplace(group->end(), value);
}

std::vector<int>* FreqTable::complement(std::vector<int> group, const size_t& n) {
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

vector<int>* FreqTable::get_single_node(std::vector<int> group, const bool *visited) {
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

