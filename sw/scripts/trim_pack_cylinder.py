#!/usr/bin/env python
import re, sys, os
import datetime
from datetime import *
import time
import math

if "PCL_VORO_PROJDIR" not in os.environ:
  subprocess.call(["mac_popup.sh",'%s: Need to source sw/sourceMe.sh' % sys.argv[0]])
  sys.exit(1)

#import sys
import subprocess
subprocess.call("_script_counter.sh %s" % " ".join(sys.argv), shell=True)

# USAGE:
#       $ trim_pack_cylinder.sh pack_cylinder -4 4 -4 4 -20 20 > pack.trimmed
in_range=1.

xmin=0
xmax=0
ymin=0
ymax=0
zmin=0
zmax=0

def usage():
  print "USAGE:",sys.argv[0]," [packing file] [xMin] [xMax] [yMin] [yMax] [zMin] [zMax]"
  print "Only allow thru x,y,z points in [input filename] that follow,"
  print "  [xMin] < x < [xMax]"
  print "  [yMin] < y < [yMax]"
  print "  [zMin] < z < [zMax]"


if os.isatty(0) == False:
  usage()
  sys.exit(1)
else:

  if len(sys.argv) < 8:
    usage()
    sys.exit(1)

  INFILE = sys.argv[1]
  xmin = float(sys.argv[2])
  xmax = float(sys.argv[3])
  ymin = float(sys.argv[4])
  ymax = float(sys.argv[5])
  zmin = float(sys.argv[6])
  zmax = float(sys.argv[7])

  with open(INFILE) as f:
    lines = f.readlines()

    # find max values
    for line in lines:
      a = re.match('(.*) (.*) (.*) (.*)',line)
      if a:
	cellId=int(a.group(1))
	x=float(a.group(2))
	y=float(a.group(3))
	z=float(a.group(4))
        if (x > xmin) and (x < xmax) and (y > ymin) and (y < ymax) and (z > zmin) and (z < zmax):
	  print cellId, x, y, z

