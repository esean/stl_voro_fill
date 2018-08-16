#!/usr/bin/env python

#
# translates input file 'pack-cube-0.6-000' to requested (x2,y2,z2) where
#	x2 = xin * xMULT
#	y2 = yin * yMULT
#	z2 = zin * zMULT
#

import re, sys, os
import datetime
from datetime import *
import time
import math
import subprocess
 
if "PCL_VORO_PROJDIR" not in os.environ:
  subprocess.call(["mac_popup.sh",'%s: Need to source sw/sourceMe.sh' % sys.argv[0]])
  sys.exit(1)

#import sys
#import subprocess
subprocess.call("_script_counter.sh %s" % " ".join(sys.argv), shell=True)

# no exponential scaling
in_range = 1.0

def usage():
  print "USAGE:",sys.argv[0]," [packing file] [xMULT] [yMULT] [zMULT]"
  print "Determines bounds[] of [packing file] and transposes points"
  print "to bounds*{xyz}MULT, or integer multiples of bounds[]"

if os.isatty(0) == False:
  usage()
  sys.exit(1)
else:

  if len(sys.argv) < 5:
    usage()
    sys.exit(1)

  INFILE = sys.argv[1]
  xMULT = float(sys.argv[2])
  yMULT = float(sys.argv[3])
  zMULT = float(sys.argv[4])

  # first find packing file ranges
  xmin=xmax=ymin=ymax=zmin=zmax=cellmax=0
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

	cellmax = max(cellmax,cellId)
	xmin = min(xmin,x)	
	xmax = max(xmax,x)
	ymin = min(ymin,y)	
	ymax = max(ymax,y)
	zmin = min(zmin,z)	
	zmax = max(zmax,z)

#  print "# x=",xmin,xmax,", y=",ymin,ymax,", z=",zmin,zmax,", cellmax=",cellmax
  xnew = (xmax-xmin)*xMULT+xmin;
  ynew = (ymax-ymin)*yMULT+ymin;
  znew = (zmax-zmin)*zMULT+zmin;
  if (xMULT==0) and (yMULT==0) and (zMULT==0):
    cellStart = 0
  else:
    cellStart = cellmax
#  print "# New (x,y,z) = (",xnew,ynew,znew,") cellStart=",cellStart

  # USAGE: xlate_pack_cylinder.py  [InputFilename] [IN_RANGE] [X_offset] [Y_offset] [Z_offset] [X_scale] [Y_scale] [Z_scale] [cellId start]

  ## call date command ##
  cmdtxt = 'xlate_pack_cylinder.py %s %f %f %f %f %f %f %f %d' % (INFILE,in_range,xnew,ynew,znew,1,1,1,cellStart)
  p = subprocess.Popen(cmdtxt, stdout=subprocess.PIPE, shell=True)
 
  ## Talk with date command i.e. read data from stdout and stderr. Store this info in tuple
  ## Interact with process: Send data to stdin. Read data from stdout and stderr, until end-of-file is reached. Wait for process to terminate. The optional input argument should be a string to be sent to the child process, or None, if no data should be sent to the child.
  (output, err) = p.communicate()
 
  ## Wait for date to terminate. Get return returncode ##
  p_status = p.wait()
  outtxt = output.strip()
  print outtxt
#  print "Command exit status/return code : ", p_status
  if p_status > 0:
    print "ERROR: error returned from following command,"
    print "      $ ",cmdtxt
    sys.exit(1)

