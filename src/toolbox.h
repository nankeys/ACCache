//
// Created by Alfred on 2022/7/22.
//

#ifndef CORANA_TOOLBOX_H
#define CORANA_TOOLBOX_H

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <queue>
#include <climits>
#include <random>
using namespace std;
using namespace chrono;

inline vector<string> split(string& str, char symbol)
{
    stringstream ss(str);
    vector<string> result;

    while (ss.good()) {
        string substr;
        getline(ss, substr, symbol);
        result.push_back(substr);
    }

    return result;
}

inline void arr_uint2char(char **dest, uint8_t **matrix, const int& row, const int& column)
{
    //dest = new char* [row];
    for(int i = 0; i < row; i ++) {
        //dest[i] = new char [column + 1];
        for(int j = 0; j < column; j ++) {
            dest[i][j] = matrix[i][j];
        }
        //dest[i][column] = '\0';
    }
}

inline bool dinicBFS(vector<vector<pair<int, int>>>& graph, vector<int>& level, int vertexNum)
{
    queue<int> q;
    for (int i = 0; i <= vertexNum; i++)
        level[i] = 0;

    q.push(1);
    level[1] = 1;
    int u, v;
    while (!q.empty()) {
        u = q.front();
        q.pop();
        for (v = 1; v <= vertexNum; v++) {
            if (!level[v] && graph[u][v].first > graph[u][v].second) {
                level[v] = level[u] + 1;
                q.push(v);
            }
        }
    }
    return level[vertexNum] != 0;
}

inline int dinicDFS(vector<vector<pair<int, int>>>& graph, vector<int>& level, int vertexNum, int currentVertex, int cp, vector<vector<bool>>& flag)
{
    int tmp = cp;
    int v, t;
    if (currentVertex == vertexNum)
        return cp;
    for (v = 1; v <= vertexNum && tmp; v++) {
        if (level[currentVertex] + 1 == level[v]) {
            if (graph[currentVertex][v].first > graph[currentVertex][v].second && flag[currentVertex][v]) {
                t = dinicDFS(graph, level, vertexNum, v,
                             min(tmp, graph[currentVertex][v].first - graph[currentVertex][v].second), flag);
                //printf("cp = %d, currentVertex = %d, first = %d, second = %d, v = %d, t = %d, tmp = %d\n", cp, currentVertex,graph[currentVertex][v].first, graph[currentVertex][v].second, v, t, tmp);
                //if(graph[currentVertex][v].first - graph[currentVertex][v].second >= tmp || tmp == cp) {
                    graph[currentVertex][v].second += t;
                    graph[v][currentVertex].second -= t;
                    tmp -= t;
                /*} else {
                    flag[currentVertex][v] = false;
                    flag[v][currentVertex] = false;
                }*/
            }
        }
    }
    return cp - tmp;
}

/*
 * pair<int, int>
 * first: capacity
 * second: flow
 */
inline int dinic(vector<vector<pair<int, int>>>& graph, int vertexNum)
{
    vector<vector<bool>> flag;
    flag.resize(vertexNum + 1);
    for(int i = 0; i < vertexNum + 1; i ++) {
        flag[i].resize(vertexNum + 1);
        for(int j  = 0; j < vertexNum + 1; j ++) {
            flag[i][j] = true;
        }
    }

    vector<int> level;
    int sum = 0, tf = 0;
    level.resize(vertexNum + 1);
    while (dinicBFS(graph, level, vertexNum))
    {
        while ((tf = dinicDFS(graph, level, vertexNum, 1, INT_MAX, flag)))
            sum += tf;
    }
    return sum;
}

// 采样字符集
static constexpr char CCH[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

// sz: 字符串的长度
// printable：是否可打印。如果用作key，可以使用不可打印的字符哟
inline string makeRandStr(int sz, bool printable)
{
    string ret;
    ret.resize(sz);
    std::mt19937 rng(std::random_device{}());
    for (int i = 0; i < sz; ++i)
    {
        if (printable)
        {
            uint32_t x = rng() % (sizeof(CCH) - 1);
            ret[i] = CCH[x];
        }
        else
        {
            ret[i] = rng() % 0xFF;
        }
    }

    return ret;
}

class timeit {
public:
    time_point<system_clock> s, e;

    void start() {
        s = system_clock::now();
    }

    void end() {
        e = system_clock::now();
    }

    double passedtime() {
        auto duration = duration_cast<microseconds>(this->e - this->s);
        double elapsed = double(duration.count()) * microseconds::period::num / microseconds::period::den;
        return elapsed;
    }
};

#endif //CORANA_TOOLBOX_H
