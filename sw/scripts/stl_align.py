#!/usr/bin/env python
import re, sys, os
import datetime
from datetime import *
import time
import math
import subprocess
import getopt as go
import python_helpers as ph

# always goes first
ph.check_basic_stuff_first(sys.argv,False)

#-----------------------------------
def usage():
  print "USAGE:",sys.argv[0]," [where to place] [STL image]"
  print
  print "Translates [STL] to align it with the (0,0,0) point."
  print
  print "Where optional {[ARGS]} can be,"
  print
  print "Required params:"
  print
  print "    [where to place] 	--> one of 'base','center','top'"
  print "    [STL image] 	--> user-provided 3d model"
  print 
  print " USAGE:"
  print 
  print " 	",sys.argv[0]," base [STL image]"
  print
  print "Writes output to '[STL image]-xlate.stl'"
  print



#-----------------------------------
#-----------------------------------
#-----------------------------------

if os.isatty(0) == False:
  usage()
  sys.exit(1)
else:

  if len(sys.argv) < 3:
    usage()
    sys.exit(1)

  argcnt = 1
  where = sys.argv[argcnt]; argcnt+=1
  print "# where to place=%s" % where
  STL_FILE = sys.argv[argcnt]; argcnt+=1
  print "# STL=%s" % STL_FILE

  if where == 'base':
    ymin = ph.run_stl_info_get_ymin(STL_FILE)
    (ret,retfn) = ph.run_translate_stl(STL_FILE,0,-1.0 * ymin,0)
  elif where == 'center':
    (cx,cy,cz) = ph.run_stl_info_get_center(STL_FILE)
    (ret,retfn) = ph.run_translate_stl(STL_FILE,-1.0 * cx,-1.0 * cy,-1.0 * cz)
  elif where == 'top':
    ymax = ph.run_stl_info_get_ymax(STL_FILE)
    (ret,retfn) = ph.run_translate_stl(STL_FILE,0,-1.0 * ymax,0)
  else:
    print "ERROR: unknown [where to place] '%s'" % where
    sys.exit(1)

  if ret is not 0:
    ph.die("Aligning to %s returned failure" % where)

  print retfn

  sys.exit(0)

