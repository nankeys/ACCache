#!/usr/bin/env python3

""" get the freqs of all groups and test multiway partition problem
"""

import prtpy
from time import perf_counter

#groupfile = "newgroup34"
#path_prefix = "/home/flnan/group_divided/"

def group_distribution(groupfile: str, bin_num: int):
    with open(groupfile,'r') as fin:
        cnts = fin.readlines()

    group_freqs = list()
    for i in range(0, len(cnts), 2):
        num, size, freq = cnts[i].strip().split("\t")
        group_freqs.append(int(freq))


    gf = dict()
    for no, freq in enumerate(group_freqs):
        gf[no] = freq


    res =  prtpy.partition(algorithm=prtpy.partitioning.greedy, numbins=bin_num, items=gf)

    for i in range(len(res)):
        res[i].sort()
    #start = perf_counter()
    #print(f"\t {perf_counter()-start} seconds")

    return res


def group_distribution2(gf: dict, bin_num: int):
    res =  prtpy.partition(algorithm=prtpy.partitioning.greedy, numbins=bin_num, items=gf)

    for i in range(len(res)):
        res[i].sort()
    #start = perf_counter()
    #print(f"\t {perf_counter()-start} seconds")

    return res

def hello123(name) -> str:
    print("pointed")
    return "Hello %s" % name

def test1(aa: dict):
    print("pointed")
    for it in aa:
        print("dict[%s] = %d" % (it, aa[it]))


if __name__ == "__main__":
    with open("/home/flnan/twitter/stat34", 'r') as fin:
        cnts = fin.readlines()

    keys = dict()
    i = 0
    for item in cnts:
        k,s,f = item.strip().split()
        keys[i] = int(f)
        i = i + 1

#    print(group_distribution2(keys, 19))

    print([1,2,3])

def test2():
    with open("/home/flnan/twitter/stat34", 'r') as fin:
        cnts = fin.readlines()

    keys = dict()
    i = 0
    for item in cnts:
        k,s,f = item.strip().split()
        keys[i] = int(f)
        i = i + 1

    return group_distribution2(keys, 19)