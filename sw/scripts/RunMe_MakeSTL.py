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
ph.check_basic_stuff_first(sys.argv)

PACK_FILE = "%s/sw/data/pack-cube" % os.environ['PCL_VORO_PROJDIR']
# pull from config file
set_face_mm = float(ph.config_read_key_value(ph.CFG_FILE,'set_face_mm'))
TUBE_RADIUS = float(ph.config_read_key_value(ph.CFG_FILE,'TUBE_RADIUS'))
TUBE_SCALE = float(ph.config_read_key_value(ph.CFG_FILE,'TUBE_SCALE'))


#-----------------------------------
def usage():
  print "USAGE:",sys.argv[0]," {[ARGS]} [STL image.stl] {[render scale factor]}"
  print
  print "Generates STL of [STL image.stl] filled with voronoi."
  print 
  print "Where optional {[ARGS]} can be,"
  print "	-p	render high-performance render for 3d printing, {[render scale factor]} must then be provided last param."
  print "       if -p is not provided, the model is run and the calculated render scale factor printed to console."
  print
  print "Required params:"
  print
  print "    [STL image] 	--> user-provided 3d model"
  print
  print " USAGE:"
  print 
  print "   $ RunMe_MakeSTL.py -p [STL image.stl] 4.777736"
  print
  print " where the '4.777736' value was computed from running the following:"
  print
  print "   $ RunMe_MakeSTL.py [STL image.stl]"
  print "     ...."
  print "   VORO_SCALING_FIT:4.777736"
  print



#-----------------------------------
#-----------------------------------
#-----------------------------------

if os.isatty(0) == False:
  usage()
  sys.exit(1)
else:

  # PARAM: {[ARGS]} [STL image.stl] [packing file] [tube radius] [tube scaling factor] {[render scale factor]}
  print "argvlen2=%d" % len(sys.argv) # TODO: remove
  if len(sys.argv) < 2:
    usage()
    sys.exit(1)

  hp_mode = ''
  argcnt = 1
  if sys.argv[argcnt] == "-p":
    hp_mode = "-p"
    argcnt+=1
    print "argvlen2=%d" % len(sys.argv)	# TODO: remove
    if len(sys.argv) < 4:
      usage()
      sys.exit(1)
  STL_FILE = sys.argv[argcnt]; argcnt+=1
  RENDER_SCALE = 0.0
  if hp_mode == "-p":
    RENDER_SCALE = float(sys.argv[argcnt]); argcnt+=1
  print "# STL=%s packing=%s tube_radius=%f tube_scaling=%f RENDER_SCALE=%f" % (STL_FILE,PACK_FILE,TUBE_RADIUS,TUBE_SCALE,RENDER_SCALE)

  # fix any errors with STL
  print "# fix any STL errors..."
  (ret,res) = ph.fix_and_correct_STL(STL_FILE)

  AVG_SIZE = set_face_mm
  print "# setting voro face_mm:%f" % (AVG_SIZE)


  # RENDER
  if hp_mode == "-p":
    # Make high-performance render for 3d-printing
    (ret,res) = ph.run_subprocess('run_voro_fill_stl.sh -p %s %s %f %f %f %f' % (STL_FILE,PACK_FILE,RENDER_SCALE,TUBE_RADIUS,TUBE_SCALE,AVG_SIZE))
    sys.exit(ret)


  # CALCULATE RENDER SCALING FACTOR
  tfn = ph.get_tempfile("xy_slope")
  xy = ph.xy_slope(tfn,100)
  result = AVG_SIZE

  # here we search for best [packing ratio] using regression, poke-method
  # the voro_fill app needs to know AVG_SIZE (cell-face mm) for creating extra outside shell
  (fmin,fmax) = ph.get_face_min_max_give_scaling(STL_FILE,PACK_FILE,result*2,TUBE_RADIUS,TUBE_SCALE,AVG_SIZE)
  avg = (fmin+fmax)/2
  print "scale=",result*2," min=",fmin," max=",fmax," avg=",avg
  xy.new_sample(result*2,avg)
  (fmin,fmax) = ph.get_face_min_max_give_scaling(STL_FILE,PACK_FILE,result,TUBE_RADIUS,TUBE_SCALE,AVG_SIZE)
  avg = (fmin+fmax)/2
  print "scale=",result," min=",fmin," max=",fmax," avg=",avg
  xy.new_sample(result,avg)

  # now we search for best setting of 'result' which gives voro face_mm of 'AVG_SIZE'
  while abs(avg - AVG_SIZE) > 0.1:
    result = xy.get_x_value_given_y(AVG_SIZE)
    (fmin,fmax) = ph.get_face_min_max_give_scaling(STL_FILE,PACK_FILE,result,TUBE_RADIUS,TUBE_SCALE,AVG_SIZE)
    avg = (fmin+fmax)/2
    print "scale=",result," min=",fmin," max=",fmax," avg=",avg
    xy.new_sample(result,avg)

  print "VORO_SCALING_FIT:%f" % result

  sys.exit(0)

