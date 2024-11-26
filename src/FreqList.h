//
// Created by Alfred on 2022/7/22.
//

#ifndef CORANA_FREQLIST_H
#define CORANA_FREQLIST_H


#include <list>
#include "ListNode.h"

class FreqList {
public:
    int lengthLimit;
    int freqLimit;
    int list_len;
    list<ListNode> distList;

    FreqList(const int& lenlim, const int& freqlim);
    list<ListNode>::iterator insert(const ListNode& ln);
    int len() const;
    list<ListNode>::iterator index(const ListNode& ln);
    bool isHot(list<ListNode>::iterator pr) const;
    void del(list<ListNode>::iterator pr);
    void clear();
    ~FreqList();
};


#endif //CORANA_FREQLIST_H
