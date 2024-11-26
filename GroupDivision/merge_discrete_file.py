#!/usr/bin/env python3

import os
os.chdir(r"/home/flnan/group_divided")

#group_num = 49177
def merge_group(traceno, fnum, subno: int, is_sub = True):
    is_subgraph = is_sub
    subno = subno
    parent_graph = "graph%02d_%s_agg" % (traceno, fnum)

    if is_subgraph:
        gprefix = "graph" + str(subno)
    else:
        gprefix = "graph%02d_%s"  % (traceno, fnum)
    group_file = gprefix  #+ r"_"+str(group_num)

    freqkeys_file = "freqkeys%02d_%s" % (traceno, 10)
    stat_file = "../twitter/stat%02d" % traceno

    if is_subgraph:
        with open(parent_graph, 'r') as fin:
            pcnts = fin.readlines()

        pgroups = list()
        for i in range(0, len(pcnts), 2):
            l, s, f = pcnts[i].strip().split('\t')
            tmp = pcnts[i + 1].strip().split('\t')
            for j in range(len(tmp)):
                tmp[j] = int(tmp[j])
            pgroups.append(tmp)

    with open(group_file, 'r') as fin:
        cnts = fin.readlines()

    with open(freqkeys_file, 'r') as fin:
        freqkeys = fin.readlines()

    with open(stat_file, 'r') as fin:
        keystat = fin.readlines()

    key_info = dict()
    tsize = 0
    tfreq = 0
    for info in keystat:
        key, size, freq = info.strip().split("\t")
        key_info[key] = (int(size), int(freq))
        tsize += int(size)
        tfreq += int(freq)

    for i in range(len(freqkeys)):
        freqkeys[i] = freqkeys[i].strip()

    dgroups = dict() #[list() for i in range(group_num)]
    gsize = list()
    gfreq = list()
    glen = list()

    for line in cnts:
        kn, n =  line.strip().split()
        if is_subgraph:
            if int(kn) not in pgroups[subno]:
                continue
        if n not in dgroups:
            dgroups[n] = list()
        dgroups[n].append(int(kn))

    groups = list()
    for d in dgroups:
        groups.append(dgroups[d])

    total_size = 0
    total_freq = 0

    for i in range(len(groups)):
        groups[i].sort()
        size = 0
        freq = 0
        for node in groups[i]:
            key = freqkeys[node]
            size += key_info[key][0]
            freq += key_info[key][1]
        total_size += size
        total_freq += freq
        gsize.append(size)
        gfreq.append(freq)
        glen.append(len(groups[i]))

    print(sum(glen))
    #print(len(pgroups[subno]))

    with open(gprefix + '_agg', 'w') as fout:
        for i in range(len(groups)):
            if(glen[i] == 0): continue
            fout.write("%d\t%d\t%d\n" % (glen[i], gsize[i], gfreq[i]))
            for node in groups[i]:
                fout.write(str(node)+'\t')
            fout.write('\n')

if __name__ == "__main__":
    #for i in [10, 80, 126, 170]:
    #    merge_group(i, True)
    # traceno = 23
    # # fnum = "50"
    # fnum = [10, 50, 100, 300, 500]
    # for i in fnum:
    #     merge_group(traceno, str(i), 1, False)
    merge_group(23, 10, 1, False)