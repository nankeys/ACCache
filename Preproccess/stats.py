
day = 0
stat = dict()
th = 0
lnum = 0

with open("workload23_" + str(day), "r") as f:
    line = f.readline()
    while(line != ""):
        cnts = line.split(",")
        if(cnts[1] in stat):
            stat[cnts[1]][0] +=1
        else:
            attr = list()
            attr.append(1)
            attr.append(cnts[3])
            stat[cnts[1]] = attr
        line = f.readline()
print("read complete\n")

with open("stat23", "w") as f:
    for item in stat.items():
        f.write("%s\t%d\t%d\n" % (item[0], int(item[1][1]), int(item[1][0])))
