import os

fname = "cluster25."
fprefix = "workload25"
tstart = 0
dlong = 60 * 60 * 24
day = 0

for f in range(0, 5):
	with open(fname+("%03d" % f), 'r') as fin:
	    print("day =",day)
	    fout = open(fprefix+str(day),'a')
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
	            fout = open(fprefix+str(day),'a')
	            print("day =",day)
	            continue
	        if day > 10:
	            break
	        line = fin.readline()

	    fout.close()
	os.remove(fname+("%03d" %f))
