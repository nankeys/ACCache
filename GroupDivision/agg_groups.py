
"""
生成合并的graph file
"""
import os
os.chdir(r"/home/flnan/group_divided")
level = 1
nnode = 19
group_no = 16

group_file = r"graph34_"+str(group_no)+"_l" + str(level)


class cgroup:
    def __init__(self, num, size, freq, nodes):
        self.num = num
        self.size = size
        self.freq = freq
        self.nodes = nodes

    def __add__(self, __o):
        num = self.num + __o.num
        size = self.size + __o.size
        freq = self.freq + __o.freq
        nodes = self.nodes + __o.nodes
        return cgroup(num, size, freq, nodes)


with open(group_file, 'r') as fin:
    cnts = fin.readlines()

groups = list()
tfreq = 0
for i in range(0, len(cnts), 2):
    #print(i,cnts[i])
    no, n, s, f = cnts[i].strip().split('\t')
    tfreq += int(f)
    i = i + 1
    #print(i)
    nodes = cnts[i].split('\t')
    nodesn = list()
    for i in range(int(n)):
        nodesn.append(int(nodes[i]))
    tmpg = cgroup(int(n), int(s), int(f), nodesn)
    groups.append(tmpg)

cnts = list()
avgfreq = tfreq / 20
print(tfreq, avgfreq)

ngroups = list()
ngnum = list()
nn = list()
tmpg = cgroup(0, 0, 0, list())
for i in range(len(groups)):
    #print(type(tmpg))
    if tmpg.freq < avgfreq and groups[i].freq < avgfreq and groups[i].num < 1000:
        tmpg = tmpg + groups[i]
        nn.append(i)
    if tmpg.freq > avgfreq:
        ngroups.append(tmpg)
        tmpg = cgroup(0, 0, 0, list())
        ngnum.append(nn)
        nn = list()
    if groups[i].freq > avgfreq:
        if (tmpg.nodes != None):
            ngroups.append(tmpg)
        ngroups.append(groups[i])
        tmpg = cgroup(0, 0, 0, list())
        ngnum.append(nn)
        nn = list()
ngroups.append(tmpg)
ngnum.append(nn)

with open("graph"+str(group_no)+"l" + str(level) + "_agg", 'w') as fout:
    fout.write(str(len(ngroups)))
    fout.write('\n')
    for g in ngroups:
        pass
        print(g.num, g.freq)
        print(g.nodes)
        fout.write(str(g.num) + '\t' + str(g.size) + '\t' + str(g.freq) + '\n')
        for i in g.nodes:
            fout.write(str(i) + '\t')
        fout.write('\n')
# with open("graph"+str(group_no)+"l" + str(level) + "_agg", 'w') as fout:
#     for n in ngnum:
#         for i in n:
#             fout.write(str(i) + '\t')
#         fout.write('\n')


