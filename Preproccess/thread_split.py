traceno = 25
day = 0
nthreads = 128
inprefix = "workload"

count = -1
for count,line in enumerate(open(inprefix+("%02d_%d" % (traceno, day)),'r')):
    count += 1

nlines = int(count / nthreads) + 1

with open(inprefix+("%02d_%d" % (traceno, day)),'r') as fin:
    for t in range(nthreads):
        outfile = "t%dd%dt%dp%04d" % (traceno, day, nthreads, t)
        fout = open(outfile, 'w');
        for lnum in range(nlines):
            cnt = fin.readline()
            fout.write(cnt)
        fout.close()
