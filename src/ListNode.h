//
// Created by Alfred on 2022/7/22.
//

#ifndef UNTITLED_LISTNODE_H
#define UNTITLED_LISTNODE_H

#include <string>
using namespace std;

const int MAX_INT = 999999;

class ListNode {
public:
    int first, second;
    int freq;
    ListNode(const ListNode& ln);
    ListNode(const int& a, const int& b);
    bool samepair(const ListNode& ln) const;
    ListNode operator+(const ListNode& ln) const;
    bool operator==(const ListNode& ln) const;
    string toString() const;
    ~ListNode();
};


#endif //UNTITLED_LISTNODE_H
