#!/usr/bin/env python
import matplotlib.pyplot as plt
import pandas as pd
import sys
if len(sys.argv) <= 2:
  print
  print "USAGE:",sys.argv[0],"{[-i]} [CSV file] [column to plot]"
  print
  print "CSV file headers should not have leading #"
  print
  print "{[-i]} just shows avg and stddev"
  print
  sys.exit(1)

#import sys
import subprocess
subprocess.call("_script_counter.sh %s" % " ".join(sys.argv), shell=True)

ac=1
bInfo = False
if sys.argv[ac] == "-i":
	ac+=1
	bInfo = True
csvfn = sys.argv[ac]
ac+=1
colnm = sys.argv[ac]
ac+=1
data = pd.read_csv(csvfn, quoting=2)
data.hist(bins=100)
if bInfo:
	avg=data.mean()
	stddev=data.std()
	print "# avg,stddev"
	print "%f,%f" % (avg,stddev)
	sys.exit()
plt.xlim([min(data[colnm]),max(data[colnm])])
plt.title("Data")
plt.xlabel("%s in %s" % (colnm,csvfn))
plt.ylabel("Frequency")
plt.show()

