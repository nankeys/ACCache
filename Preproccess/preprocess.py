import os
import json

def split_trace_in_days(traceno, path):
    tstart = 0
    dlong = 60 * 60 * 24
    day = 0
    
    fprefix = "workload%d_" % traceno
    fname = "cluster%d.sort" % traceno
    
    stat = dict()

    with open(path + '/' + fname, 'r') as fin:
        print("day =", day)
        fout = open(path + '/' + fprefix + str(day), 'a')
        line = fin.readline()
        while(line != ''):
            cnts = line.strip().split(',')
            ts = int(cnts[0])
            #if ts % 3600 == 0 and ts >= 3600:
            #    print(ts)
            if ts < (day+1) * dlong:
                fout.write(line)
                if(day == 0 and cnts[1] in stat):
                        stat[cnts[1]][0] +=1
                elif(day == 0):
                    attr = list()
                    attr.append(1)
                    attr.append(cnts[3])
                    stat[cnts[1]] = attr
            else:
                fout.close()
                if(day == 0):
                    with open(path + '/' + "stat{}".format(traceno), "w") as f:
                        for item in stat.items():
                            f.write("%s\t%d\t%d\n" % (item[0], int(item[1][1]), int(item[1][0])))
                    stat.clear()
                    stat = dict()
                    print("""The stat file has been generated at {},
        whose name is stat{}""".format(path, traceno))
                day += 1
                fout = open(path + '/' + fprefix + str(day),'a')
                print("day =",day)
                continue
            if day > 10:
                break
            line = fin.readline()

        fout.close()
    print("""The trace has been split by day,
        which is generated under {} with prefix 'workload{}'""".format(path, traceno))

def meta_generate_stats(traceno, path):
    day = 0
    stat = dict()

    cpath = path + '/' + "kvcache{}".format(traceno)

    with open(cpath + '/' + "kvcache_traces_%d.csv" % (day + 1), "r") as f:
        line = f.readline()
        while(line != ""):
            cnts = line.split(",")
            if(cnts[0] == 'op_time'):
                line = f.readline()
                continue
            if(cnts[1] in stat):
                stat[cnts[1]][0] +=1
            else:
                attr = list()
                attr.append(1)
                attr.append(cnts[5])
                stat[cnts[1]] = attr
            line = f.readline()
    print("read complete\n")

    with open(path + '/' + "stat%d" % traceno, "w") as f:
        for item in stat.items():
            f.write("%s\t%d\t%d\n" % (item[0], int(item[1][1]), int(item[1][0])))

def extract_frequency(traceno, path, locl):

    statfile = path + '/' + "stat{}".format(traceno)
    with open(statfile, 'r') as f:
        contents = f.readlines()

    lfreq = list()
    for line in contents:
        key, size, freq = line.strip().split('\t')
        size = int(size)
        freq = int(freq)
        lfreq.append(freq)

    lfreq.sort(reverse=True)
    flen = len(lfreq)

    sflen = sum(lfreq)

    print("aver freq = ", sflen / flen)

    for i in [10, 30, 50, 70, 100]:
        if (i * 1000 >= flen): break
        freq = lfreq[i * 1000]
        j = 1
        while (lfreq[i * 1000 + j] == freq):
            j = j + 1
        locl.append(freq)
        # print("loc[%dk] = loc[%d] = %d" % (i, i * 1000 + j - 1, freq))

    total = 0
    loc = 0
    for i in range(flen):
        total += lfreq[i]
        if(total / sflen >= 0.8):
            # print("80%% hotness is at %d" % i)
            loc = i
            break

    # print("loc[%d] = %d" % (loc, lfreq[loc]))
    locl.append(lfreq[loc])

    print(locl)
    return locl[2]

# load JSON file
def read_json_file(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)
    return data
        
def update_content(config_file, trace_type, hotlim):
    with open('../src/correltion.ori', 'r') as f:
        content = f.read()
        
    content = content.format(trace_type)
    
    with open('../src/main_correlation.cpp', 'w') as f:
        f.write(content)

    with open('../src/main.ori', 'r') as f:
        content = f.read()
        
    content = content.format(trace_type)
    
    with open('../src/main.cpp', 'w') as f:
        f.write(content)
        
    data = read_json_file(config_file)
    data['hotest_freq_limit'] = hotlim
    
    with open(config_file, "w") as json_file:
        json.dump(data, json_file)

def get_input(promt, default):
    value = input(promt + f" (default: {default}): \n")
    if value == '':
        return default
    else:
        return value
        
if __name__ == '__main__':
    config_file = '../src/config.json'
    data = read_json_file(config_file)
    traceno = data['trace_no']
    path = data['path_prefix']
    trace_type = data['trace_type']
    
    if(trace_type == 'twitter'):
        locl = list()
        
        split_trace_in_days(traceno, path)
    else:
        meta_generate_stats(traceno, path)
        
    hotlim = extract_frequency(traceno, path, locl)
    update_content(config_file, trace_type, hotlim)

