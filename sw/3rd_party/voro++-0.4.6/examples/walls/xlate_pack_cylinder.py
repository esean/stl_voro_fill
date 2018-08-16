#!/usr/bin/env python
import re, sys, os
import datetime
from datetime import *
import time
import math

# USAGE:
#       $ ./xlate_pack_cylinder.sh 1 | grep -v ^# > pc
in_range=1.

INFILE="pack_cylinder"
#INFILE="pack_cylinder_small"
xd=0.0
xm=0.6
yd=0.0
ym=0.6
zd=17.25
zm=0.6

pwroot=500.0


pipe_in = False
if os.isatty(0) == False:
  pipe_in = True
  print "USAGE:",sys.argv[0]," [IN_RANGE]"
  sys.exit(1)
else:

  if len(sys.argv) <= 1:
    print "USAGE:",sys.argv[0]," [IN_RANGE]"
    sys.exit(1)
  in_range = float(sys.argv[1])

  xmax = ymax = zmax = 0.
  with open("pack_cylinder") as f:
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
	xval=float(a.group(2))
	yval=float(a.group(3))
	zval=float(a.group(4))
	xabs=abs(xval)
	yabs=abs(yval)
	zabs=abs(zval)
        if in_range == 1.0:
	  gx = (xval - xd) * xm
          gy = (yval - yd) * ym
          gz = (zval - zd) * zm
	else:
	  xpwr = pwroot ** (xabs - (in_range * xmax))
	  ypwr = pwroot ** (yabs - (in_range * ymax))
	  zpwr = pwroot ** (zabs - (in_range * zmax))
	  scx = max(xm,xm * xpwr)
	  scy = max(ym,ym * ypwr)
	  #scz = max(zm,zm * zpwr)
	  scz = zm
	  gx = (xval - xd) * scx
	  gy = (yval - yd) * scy
	  gz = (zval - zd) * scz
	print a.group(1), gx, gy, gz

