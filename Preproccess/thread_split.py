import os
import json

# load JSON file
def read_json_file(file_path):
    with open(file_path, 'r') as file:
        data = json.load(file)
    return data

config_file = '../src/config.json'
data = read_json_file(config_file)
nthreads = data['thread_num']
traceno = data['trace_no']
pathprefix = data['path_prefix']
day = data['day']
trace_type = data['trace_type']



if(trace_type == 'twitter'):
    inprefix = "workload"
    count = -1
    for count,line in enumerate(open(pathprefix + '/' + inprefix+("%02d_%d" % (traceno, day)),'r')):
        count += 1

    nlines = int(count / nthreads) + 1

    with open(pathprefix + '/' + inprefix+("%02d_%d" % (traceno, day)),'r') as fin:
        for t in range(nthreads):
            outfile = "t%dd%dt%dp%04d" % (traceno, day, nthreads, t)
            fout = open(pathprefix + '/' + outfile, 'w')
            for lnum in range(nlines):
                cnt = fin.readline()
                fout.write(cnt)
            fout.close()
else:
    inprefix = "workload"
    count = -1
    for count,line in enumerate(open('{}/kvcache{}/kvcache_traces_{}'.format(pathprefix, traceno, day+1),'r')):
        count += 1

    nlines = int(count / nthreads) + 1

    with open('{}/kvcache{}/kvcache_traces_{}'.format(pathprefix, traceno, day+1),'r') as fin:
        for t in range(nthreads):
            outfile = "t%dd%dt%dp%04d" % (traceno, day, nthreads, t)
            fout = open(pathprefix + '/' + outfile, 'w')
            for lnum in range(nlines):
                cnt = fin.readline()
                if(cnt.strip().split(',')[0] in ['key', 'op_time']): continue
                fout.write(cnt)
            fout.close()