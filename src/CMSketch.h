//
// Created by Alfred on 2022/7/26.
//

#ifndef CORANA_CMSKETCH_H
#define CORANA_CMSKETCH_H


#include <vector>
#include <string>

class CMSketch {
public:
    size_t m;
    int k;
    long double epsilon;
    double delta;
    std::vector<std::vector<int> > bucket;
    long long n;

    CMSketch(const long long& n, const int& deviation, const double& confidence = 0.01);
    CMSketch(CMSketch& cms);
    CMSketch(){};
    void add(const int& a, const int& b);
    int estimateFrequency(const int& a, const int& b);
    void formalized();
    int find(const int& a, const int& b);
    bool cutGraph();
    void write2File(const std::string& filename);
    void load(const std::string& filename);
    ~CMSketch();



private:
    int *a;
    int *b;
    int *c;
    int hash(const int& a, const int& b, int& i);

    static void group_insert(std::vector<int>* group, const int& value);
    int getConnection(const std::vector<int> &group, const int& n, bool *visited);
    int getInnerConnection(const std::vector<int> &group);
    static std::vector<int>* complement(std::vector<int> group, const int& n);
    std::vector<int>* get_single_node(std::vector<int> group, const bool visited[]);
};


#endif //CORANA_CMSKETCH_H
