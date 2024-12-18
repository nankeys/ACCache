import sys

#traceno = int(sys.argv[1])
# traceno = 1

#
# pathprefix = "/home/flnan/twitter"# "/data/kvcache%d" % traceno
traceno = int(input("please input the trace No. you want to process.\n"))
statfile = input("please input the /path/to/stat%02d\n" % traceno)

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

locl = list()
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
print("""Please copy the data from the above list to the line 
      in the two-dimensional array HOTLIM corresponding to
      the array TRACENO in the file src/parameter.h,
      making traceno and hotlim correspond to each other.""")
