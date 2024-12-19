import os


traceno = int(input('Please input the trace No. you want to test.\n'))
path = input('Please input the path to cluster%d.sort, prefix only.\n' % traceno)
fprefix = "workload%d_" % traceno
fname = "cluster%d.sort" % traceno

tstart = 0
dlong = 60 * 60 * 24
day = 0

with open(path + '/' + fname, 'r') as fin:
	print("day =",day)
	fout = open(path + '/' + fprefix + str(day),'a')
	line = fin.readline()
	while(line != ''):
		cnts = line.strip().split(',')
		ts = int(cnts[0])
		#if ts % 3600 == 0 and ts >= 3600:
		#    print(ts)
		if ts < (day+1) * dlong:
			fout.write(line)
		else:
			fout.close()
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