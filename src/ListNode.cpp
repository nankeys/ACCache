//
// Created by Alfred on 2022/7/22.
//

#include "ListNode.h"
#include <exception>

struct WRONG_VALUE_ERR : public exception
{
    const char * what () const throw ()
    {
        return "not the same value";
    }
};

ListNode::ListNode(const ListNode &ln) {
    this->first = ln.first;
    this->second = ln.second;
    this->freq = ln.freq;
}

ListNode::ListNode(const int& a, const int& b) {
    if (a < b) {
        this->first = a;
        this->second = b;
    } else {
        this->first = b;
        this->second = a;
    }
    this->freq = 1;
}

bool ListNode::samepair(const ListNode &ln) const {
    return (this->first == ln.first) && (this->second == ln.second);
}

ListNode ListNode::operator+(const ListNode &ln) const {
    if(!this->samepair(ln)) {
        //return null;
        throw WRONG_VALUE_ERR();
    }

    ListNode l(this->first, this->second);

    l.freq ++;

    return l;
}

bool ListNode::operator==(const ListNode &ln) const {
    return this->samepair(ln);
}

string ListNode::toString() const {
    return to_string(this->first) + "," + ::to_string(this->second);
}

ListNode::~ListNode() = default;


