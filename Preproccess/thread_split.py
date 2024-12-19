nthreads = 128
inprefix = "workload"
traceno = int(input('Please input the Trace No. you want to test.\n'))
pathprefix = input('Please input the path where the workload is, prefix only.\n')
day = input('Please input the day of the trace you want to test (from 1-7).\n')

count = -1
for count,line in enumerate(open(pathprefix + '/' + inprefix+("%02d_%d" % (traceno, day)),'r')):
    count += 1

nlines = int(count / nthreads) + 1

with open(pathprefix + '/' + inprefix+("%02d_%d" % (traceno, day)),'r') as fin:
    for t in range(nthreads):
        outfile = "t%dd%dt%dp%04d" % (traceno, day, nthreads, t)
        fout = open(outfile, 'w')
        for lnum in range(nlines):
            cnt = fin.readline()
            fout.write(cnt)
        fout.close()
