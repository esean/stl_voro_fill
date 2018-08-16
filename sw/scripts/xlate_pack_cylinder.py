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
#       $ xlate_pack_cylinder.sh 1 | grep -v ^# > pc
in_range=1.

xd=0.0
xm=1	# all scaling previously was 0.6
yd=0.0
ym=1
zd=0.0	# was -15
zm=1
cellStart=0

pwroot=10.0	# was 500

def usage():
  print "USAGE:",sys.argv[0]," [packing file] [IN_RANGE] [X_offset] [Y_offset] [Z_offset] [X_scale] [Y_scale] [Z_scale] [cellId start]"
  print "Allows translating and scaling an [input filename] by {XYZ}_offset and {XYZ}_scale."
  print "[IN_RANGE] allows cells further from center to experience exponential outward placement."
  print "And [cellId start] provides cellId where to start, or it will start at 0 if {XYZ}_offset is 0,0,0"
  print "Math is: {xyz} = ({xyz} + {xyz}_offset) * {xyz}_scale"

if os.isatty(0) == False:
  usage()
  sys.exit(1)
else:

  if len(sys.argv) < 10:
    usage()
    sys.exit(1)
  INFILE = sys.argv[1]
  in_range = float(sys.argv[2])
  xd = float(sys.argv[3])
  yd = float(sys.argv[4])
  zd = float(sys.argv[5])
  xm = float(sys.argv[6])
  ym = float(sys.argv[7])
  zm = float(sys.argv[8])
  cellStart = int(sys.argv[9])

  xmax = ymax = zmax = 0.
  with open(INFILE) as f:
    lines = f.readlines()

    # find max values
    for line in lines:
      a = re.match('(.*) (.*) (.*) (.*)',line)
      if a:
	xmax = max(xmax,float(a.group(2)))
	ymax = max(ymax,float(a.group(3)))
	zmax = max(zmax,float(a.group(4)))
#    print "#",xmax,ymax,zmax

    # process
    for line in lines:
      a = re.match('(.*) (.*) (.*) (.*)',line)
      if a:
	cellId=int(a.group(1))+cellStart
	xval=float(a.group(2))
	yval=float(a.group(3))
	zval=float(a.group(4))
	xabs=abs(xval)
	yabs=abs(yval)
	zabs=abs(zval)
        if in_range == 1.0:
	  gx = (xval + xd) * xm
          gy = (yval + yd) * ym
          gz = (zval + zd) * zm
	else:
	  xpwr = pwroot ** (xabs - (in_range * xmax))
	  ypwr = pwroot ** (yabs - (in_range * ymax))
	  zpwr = pwroot ** (zabs - (in_range * zmax))
	  scx = max(xm,xm * xpwr)
	  scy = max(ym,ym * ypwr)
	  scz = max(zm,zm * zpwr)
	  #scz = zm
	  gx = (xval + xd) * scx
	  gy = (yval + yd) * scy
	  gz = (zval + zd) * scz
	print cellId, gx, gy, gz

