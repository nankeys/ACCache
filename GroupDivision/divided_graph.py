"""
找出超过平均值的group，生成单独的文件
"""

import os
import merge_discrete_file
import agg
import time

os.chdir(r"/home/flnan/group_divided")
node_num = 6

traceno = 23
fnum = "10k"

graph_file = "graph%02d_%s_agg" % (traceno, fnum)  #"graph_n19l" + str(level)

freqkeys_file = "freqkeys%02d_%s" % (traceno, fnum)
stat_file = "../twitter/stat%02d" % traceno

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


def split(source, g, favg):
    with open(source, 'r') as fin:
        cnts = fin.readlines()

    sgroups = list()
    sgsize = list()
    sgfreq = list()

    dgroups =  list()
    for i in range(0, len(cnts), 2):
        if g == i/2:
            n, s, f = cnts[i].strip().split('\t')
            dgroups = cnts[i + 1].strip().split('\t')

    tmp = list()
    stsize = 0
    stfreq = 0
    for item in dgroups:
        key = freqkeys[int(item)]
        isize = key_info[key][0]
        ifreq = key_info[key][1]
        if(stfreq + ifreq < favg):
            stfreq += ifreq
            stsize += isize
            tmp.append(item)
        else:
            sgroups.append(tmp)
            sgsize.append(stsize)
            sgfreq.append(stfreq)
            tmp = list()
            stsize = 0
            stfreq = 0
            stfreq += ifreq
            stsize += isize
            tmp.append(item)

    sgroups.append(tmp)
    sgsize.append(stsize)
    sgfreq.append(stfreq)
    print(sgsize)
    print(sgfreq)
    print(sgroups)

    with open(source, 'w') as fout:
        for i in range(len(sgroups)):
            fout.write("%d\t%d\t%d\n" % (len(sgroups[i]), sgsize[i], sgfreq[i]))
            for item in sgroups[i]:
                fout.write(str(item)+'\t')
            fout.write('\n')



#while(True):
with open(graph_file, 'r') as fin:
    cnts = fin.readlines()

group_num = int(len(cnts)/2)
#print(group_num)

groups = list()  #[list() for i in range(group_num)]

for i in range(0, len(cnts), 2):
    n, s, f = cnts[i].strip().split('\t')
    tmp = cnts[i+1].strip().split('\t')
    groups.append([int(i) for i in tmp])

for i in range(group_num):
    groups[i].sort()
    #print(i, groups[i])

total_size = 0
total_freq = 0
size_stat = list()
freq_stat = list()
group_len = list()
#with open(group_file, 'w') as fout:
for i in range(group_num):
    size = 0
    freq = 0
    for node in groups[i]:
        key = freqkeys[node]
        size += key_info[key][0]
        freq += key_info[key][1]
    total_size += size
    total_freq += freq
    size_stat.append(size)
    freq_stat.append(freq)
    group_len.append(len(groups[i]))

Favg = total_freq/node_num
print("Favg =", Favg)
print("Total items =", sum(group_len))

for j in range(5):
    divided_group = list()
    for i in range(group_num):
        if freq_stat[i] > Favg:
            print(i, freq_stat[i])
            divided_group.append(i)
            g = set(groups[i])
            with open("graph_"+str(i), 'w') as fout, open("louvain%02d_%s"  % (traceno, fnum), 'r') as fin:
                line = fin.readline()
                while line != "":
                    first, second, tmp = line.strip().split('\t')
                    if int(first) in g and int(second) in g:
                        fout.write(line)
                    line = fin.readline()
        else:
            continue

    for g in divided_group:
        status = os.system("louvain-convert -i graph_"+ str(g) +" -o graph"+ str(g) +".bin -w graph"+ str(g) +".w")
        time.sleep(1)
        status = os.system("louvain-louvain graph"+ str(g) +".bin -l -1 -q id_qual -w graph"+ str(g) +".w > graph"+ str(g) +".tree")
        time.sleep(1)
        status = os.system("louvain-hierarchy graph"+ str(g) +".tree -m > graph"+ str(g))
        time.sleep(1)

    #status = os.popen("rm *.bin *.w *.tree graph_*")

    for g in divided_group:
        merge_discrete_file.merge_group(traceno, fnum, g)

    agg.agg_file(divided_group, 'graph%02d_%s_agg' % (traceno, fnum), 'graph%02d_%s_agg' % (traceno, fnum))


# 顽固部分
for g in divided_group:
    split('graph%02d_%s_agg'  % (traceno, fnum), g, Favg)

# agg.agg_file(divided_group)
#print(divided_group)
