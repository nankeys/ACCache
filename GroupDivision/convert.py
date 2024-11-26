import os

os.chdir(r"E:\graph")

with open("graph_16.data", 'r') as fin, open("graph16.d", 'w') as fout:
    line = fin.readline()
    while (line != ""):
        cnts = line.strip().split('\t')
        fout.write('"%s"\t"%s"\t%s\n' % (cnts[0], cnts[1], cnts[2]))
        line = fin.readline()