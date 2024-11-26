"""
把分group合并为一个
"""

import os
#os.chdir(r"E:\graph")

def agg_file(group: list, source = 'graph02_10k_agg', dest="graph02_10k_agg"):
    tgroupf = source

    with open(tgroupf, 'r') as fin:
        cnts = fin.readlines()

    groups = list()
    for i in range(0, len(cnts), 2):
        n, s, f = cnts[i].strip().split('\t')
        no = int(i/2)
        if no in group:
            #del cnts[i]
            #del cnts[i+1]
            with open('graph' + str(int(no)) + '_agg') as fin:
                #cntst = fin.readline()
                cntss = fin.readlines()
            #print(cntss)
            groups += cntss
        else:
            stri = "%s\t%s\t%s\n" % (n, s, f)
            #print(stri)
            groups.append(stri)
            i = i + 1
            groups.append(cnts[i])

    with open(dest, 'w') as fout:
        for line in groups:
            #print(line)
            fout.write(line)

if __name__ == "__main__":
    traceno = 23
    fnum = "10k"
    agg_file([0,], 'graph%02d_%s_agg' % (traceno, fnum), 'graph%02d_%s_agg' % (traceno, fnum))