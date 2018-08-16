#!/usr/bin/env python
import re, sys, os
import python_helpers as ph

# always goes first
ph.check_basic_stuff_first(sys.argv,False)

#-----------------------------------
def usage():
  print "USAGE:",sys.argv[0]," [STL image]"
  print
  print "Display volume of [STL]"
  print
  print "Required params:"
  print
  print "    [STL image] 	--> user-provided 3d model"
  print 
  print " USAGE:"
  print 
  print " 	",sys.argv[0]," [STL image]"
  print
  print "Read-only - does not write an output"
  print



#-----------------------------------
#-----------------------------------
#-----------------------------------

if os.isatty(0) == False:
  usage()
  sys.exit(1)
else:

  if len(sys.argv) < 2:
    usage()
    sys.exit(1)

  argcnt = 1
  STL_FILE = sys.argv[argcnt]; argcnt+=1
  print "# STL=%s" % STL_FILE

  vol = ph.get_stl_volume_in_mL(STL_FILE)
  print "VOLUME_ML=%f" % vol

  sys.exit(0)

