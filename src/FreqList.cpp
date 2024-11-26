//
// Created by Alfred on 2022/7/22.
//

#include <iostream>
#include <chrono>
#include "FreqList.h"
//#include "toolbox.h"
using namespace chrono;

FreqList::FreqList(const int& lenlim, const int& freqlim) {
    this->lengthLimit = lenlim;
    this->freqLimit = freqlim;
    this->list_len = 0;
    this->distList= list<ListNode>();
}

int FreqList::len() const {
    return list_len;
}

list<ListNode>::iterator FreqList::index(const ListNode &ln) {
    //cout << "List size = " << this->distList.size() << endl;
    for(auto it = this->distList.begin(); it != this->distList.end(); it ++) {
        if(it->samepair(ln)) {
            return it;
        }
    }
    return this->distList.end();
}

bool FreqList::isHot(list<ListNode>::iterator pr) const {
    return pr->freq >= this->freqLimit;
}

list<ListNode>::iterator FreqList::insert(const ListNode &ln) {
    time_point<system_clock> s, e;
    auto it = this->index(ln);
    if(it != this->distList.end()) {
        //*it = *it + ln;
        it->freq ++;
        return it;
    } else {
        s = system_clock::now();
        this->distList.emplace_front(ln);
        this->list_len ++;
        if (this->list_len > this->lengthLimit) {
            this->distList.erase(--it);
            this->list_len --;
        }
        e = system_clock::now();
        auto duration = duration_cast<microseconds>(e - s);
        double elapsed = double(duration.count()) * microseconds::period::num / microseconds::period::den;
        //cout << "Insert time = " << elapsed << endl;
    }
    return this->distList.begin();
}

void FreqList::del(list<ListNode>::iterator pr) {
    this->distList.erase(pr);
    this->list_len --;
}

void FreqList::clear() {
    this->distList.clear();
}

FreqList::~FreqList() {
    this->clear();
}




