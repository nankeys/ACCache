//
// Created by Alfred on 2022/7/22.
//

#ifndef CORANA_FREQTABLE_H
#define CORANA_FREQTABLE_H

#include <vector>
#include <string>
#include "buffer.h"
using namespace std;

class FreqTable {
public:
    vector<int> ftable;
    size_t n;

    explicit FreqTable(const size_t& length);
    FreqTable(const FreqTable& _a);

    FreqTable();

    size_t loc(const int& a, const int &b) const;
    void add(const int& a, const int& b, const int& num = 1);
    int estimateFrequency(const int& a, const int& b);
    int at(const int& index);
    int find(const int& a, const int& b);
    bool cutGraph();
    void write2File(const std::string& filename);
    void write4louvain(const std::string& filename);
    void load(const std::string& filename);

private:
    static void group_insert(std::vector<int>* group, const size_t& value);
    int getConnection(const std::vector<int> &group, const size_t& n, bool *visited);
    int getInnerConnection(const std::vector<int> &group);
    static std::vector<int>* complement(std::vector<int> group, const size_t& n);
    std::vector<int>* get_single_node(std::vector<int> group, const bool visited[]);
};


#endif //CORANA_FREQTABLE_H
