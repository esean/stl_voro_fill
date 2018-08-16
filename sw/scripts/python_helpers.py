#!/usr/bin/env python
import re, sys, os
import time
import subprocess
import socket
import os.path
import shutil
# uncomment to debug in gdb-style debugger
#import pdb
#pdb.set_trace()

ADMIN_EMAIL = "seanharre@gmail.com" # todo: fix this
DEBUG = 0

# hard-coded should be in same dir
CFG_FILE = "build_cfg.ini"


#-----------------------------------
def die(str):
    msg_txt = "FATAL ERROR: %s !\n\nPATH = %s\n\n" % (str,os.environ['PWD'])
    print "\n%s\n" % msg_txt
    sys.exit(1)
def warn(str):
    print "\nWARNING:%s !\n" % str
def log(str):
    print "[%s]LOG:%s..." % (sys.argv[0],str)
#-----------------------------------
def rm(fn):
    if "PCL_DEBUG_NO_REMOVE" in os.environ:
        print "# PCL_DEBUG_NO_REMOVE is set: ph.rm(%s) NOT REMOVING" % fn
        return None
    if os.path.isfile(fn):
        #print "INFO: did NOT removing file:%s" % fn
        print "INFO: removing file:%s" % fn
        os.remove(fn)
    else:
        warn("ph.rm(%s) tried to remove but it didn't exist" % fn)
    return None

#-----------------------------------
def copy_file_to_dest(src,dst):
  srcbase = os.path.basename(src)
  dst_fn = "%s/%s" % (dst,srcbase)
  print "# DBG: copy_file_to_dest: cp src=%s dst=%s" % (src,dst_fn)
  if not os.path.isdir(dst):
    die("copy_file_to_dest(%s,%s) but dest is not directory" % (src,dst))
  shutil.copyfile(src,dst_fn)
  return None
#-----------------------------------
def copytree(src, dst, symlinks=False, ignore=None):
    if not os.path.exists(dst):
        os.makedirs(dst)
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            copytree(s, d, symlinks, ignore)
        else:
            if not os.path.exists(d) or os.stat(s).st_mtime - os.stat(d).st_mtime > 1:
                shutil.copy2(s, d)
#-----------------------------------
#### CONFIG (todo: make class)
#-----------------------------------
def config_read_key_value(cfgfile,key):
  if not os.path.isfile(cfgfile):
    die("config_read_key_value(%s,%s):cfg file not found" % (cfgfile,key))
  cmdtxt = "readset_inicfg.py -f %s -r %s" % (cfgfile,key)
  (ret,txt) = run_subprocess(cmdtxt)
  if ret is not 0:
      die("config_read_key_value(%s,%s):%s returned failure %d" % (cfgfile,key,cmdtxt,ret))
#  if not txt:
#      die("config_read_key_value(%s,%s) returned a NULL string" % (cfgfile,key))
#  print "# CFG: %s = %s" % (key,txt)
  return txt
#-----------------------------------
# IMPLIED IMPORTs
#import sys
#import subprocess
def check_basic_stuff_first(prog, check_for_cfg_files = True):
  global CFG_FILE
#, PW_CFG_FILE
  if "PCL_VORO_PROJDIR" not in os.environ:
    subprocess.call(["mac_popup.sh",'%s: Need to source sw/sourceMe.sh' % prog[0]])
    sys.exit(1)
  subprocess.call("_script_counter.sh %s" % " ".join(prog), shell=True)

  # we require (2) .ini files for the build, make sure we have them
  if not os.path.isfile(CFG_FILE):
    if DEBUG == 1:
        warn("local build config file %s does not exist" % CFG_FILE)
        # could not find local, if we find one at PCL_VORO_PROJDIR, warn and use that one
        base_CFG_FILE = "%s/sw/%s" % (os.environ['PCL_VORO_PROJDIR'],CFG_FILE)
        if not os.path.isfile(base_CFG_FILE):
          die("base build config file %s does not exist" % base_CFG_FILE)
        print "======================== WARNING ========================"
        warn("USING BASE CONFIG: base build config file %s exists" % CFG_FILE)
        print "======================== WARNING ========================"
        # re-assign global with base copy from svn
        CFG_FILE = base_CFG_FILE
    else:
        die("local build config file %s does not exist" % CFG_FILE)

#-----------------------------------
# returns volume in mm^3 (1mL = 1000mm^3)
#-----------------------------------
def get_stl_volume_in_mL(stl):
    # $ admesh 1in_cyl.stl | grep Volume
    # Number of parts       :     1        Volume   :  12805.110352
    cmd = "admesh %s | grep Volume | awk '{print $8}'" % stl
    (ret,res) = run_subprocess(cmd)
    if ret is not 0:
      die("get_stl_volume(%s) returned failure code %d:%s" % (stl,ret,res))
    return (float(res) / 1000.0)  # convert mm^3 to mL

#-----------------------------------
# misc_stl ali$ stl_info new_thing_1arm.stl
# DEBUGCFG:stl_info:IN STL = new_thing_1arm.stl
# DEBUG: fix_and_correct_STL(new_thing_1arm.stl)
# DEBUG:popen_cmd(fix_stl.sh new_thing_1arm.stl)
# input STL new_thing_1arm.stl is watertight
# Found 1 regions in STL
# MODEL: bounds:6.152860 20.049000 9.166360:diameter = 22.887600
# MODEL: xmin: -3.07643 xmax: 3.07643
# MODEL: ymin: -10.0245 ymax: 10.0245
# MODEL: zmin: -4.58318 zmax: 4.58318
# MODEL: center = (0.000000,0.000000,0.000000)
# MODEL: centerOfMass = (-0.107051,-1.425872,-0.864948)
# Number of points    = 4638
# Number of triangles = 9272
# Number of polys = 9272
#-----------------------------------
# Returns: more of model stl_info data
def run_stl_info(stl):
    cmd = "stl_info \"%s\"" % stl
    ymin = ymax = bx = by = bz = bdia = cx = cy = cz = None
    (ret,txt) = run_subprocess(cmd)
    if ret is not 0:
        print "ERROR: run_stl_info(%s) stl_info returned failure:%d" % (stl,ret)
        return ymin,ymax,bx,by,bz,bdia,cx,cy,cz
    for line in txt.splitlines():
        # MODEL: ymin: -10.0245 ymax: 10.0245
        a = re.match('^# MODEL: ymin: (.*) ymax: (.*)',line)
        if a:
            ymin=float(a.group(1))
            ymax=float(a.group(2))
        # MODEL: bounds:6.152860 20.049000 9.166360:diameter = 22.887600
        a = re.match('^# MODEL: bounds:(.*) (.*) (.*):diameter = (.*)',line)
        if a:
            bx=float(a.group(1))
            by=float(a.group(2))
            bz=float(a.group(3))
            bdia=float(a.group(4))
        # MODEL: center = (0.000000,0.000000,0.000000)
        a = re.match('^# MODEL: center = \((.*),(.*),(.*)\)',line)
        if a:
            cx=float(a.group(1))
            cy=float(a.group(2))
            cz=float(a.group(3))
    # done, check we found all values, or die
    if ymin == None or ymax == None or \
        bx == None or by == None or bz == None or bdia == None or \
        cx == None or cy == None or cz == None:
        die("ph.run_stl_info(%s) did not find all values in txt:%s" % (stl,txt))
    return ymin,ymax,bx,by,bz,bdia,cx,cy,cz
#-----------------------------------
# Returns: ymin value
# TODO: replace the following to use ph.run_stl_info()
def run_stl_info_get_ymin(stl):
    txt = "stl_info \"%s\" | grep '^# MODEL: ymin' | awk '{print $4}'" % stl
    (ret,res) = run_subprocess(txt)
    if ret is not 0:
        print "ERROR: run_stl_info_get_ymin(%s) stl_info returned failure:%d" % (stl,ret)
    return float(res)
#-----------------------------------
# Returns: ymax value
# TODO: replace the following to use ph.run_stl_info()
def run_stl_info_get_ymax(stl):
    txt = "stl_info \"%s\" | grep '^# MODEL: ymin' | awk '{print $6}'" % stl
    (ret,res) = run_subprocess(txt)
    if ret is not 0:
        print "ERROR: run_stl_info_get_ymax(%s) stl_info returned failure:%d" % (stl,ret)
    return float(res)
#-----------------------------------
# Returns: ymin & ymax value
# TODO: replace the following to use ph.run_stl_info()
def run_stl_info_get_ymin_ymax(stl):
    txt = "stl_info \"%s\" | grep '^# MODEL: ymin' | awk '{print $4\" \"$6}'" % stl
    (ret,res) = run_subprocess(txt)
    if ret is not 0:
        print "ERROR: run_stl_info_get_ymin_ymax(%s) stl_info returned failure:%d" % (stl,ret)
    resV = res.split(' ')
    ymin = float(resV[0])
    ymax = float(resV[1])
    return (ymin,ymax)
#-----------------------------------
# TODO: replace the following to use ph.run_stl_info()
def run_stl_info_get_bounds(stl):
    txt = "stl_info \"%s\" | grep '^# MODEL: bounds:' | cut -d: -f3" % stl
    (ret,res) = run_subprocess(txt)
    resV = res.split(' ')
    bx = float(resV[0])
    by = float(resV[1])
    bz = float(resV[2])
    return (bx,by,bz)
#-----------------------------------
# TODO: replace the following to use ph.run_stl_info()
def run_stl_info_get_bounds_diameter(stl):
    txt = "stl_info \"%s\" | grep '^# MODEL: bounds:' | cut -d: -f4 | awk '{print $3}'" % stl
    (ret,res) = run_subprocess(txt)
    return float(res)
#-----------------------------------
# TODO: replace the following to use ph.run_stl_info()
def run_stl_info_get_center(stl):
    # MODEL: center = (0.759800,-0.001800,-1.158500)
    txt = "stl_info \"%s\" | grep '^# MODEL: center = ' | tr -d '[\(\)]' | awk '{print $5}'" % stl
    (ret,res) = run_subprocess(txt)
    resV = res.split(',')
    bx = float(resV[0])
    by = float(resV[1])
    bz = float(resV[2])
    return (bx,by,bz)

#-----------------------------------
# RETURNS 0 on success
def run_subprocess(cmdtxt,runCmdInBackground = False):
  print "\n# DBG:SUBPROCESS:%s(%d)" % (cmdtxt,runCmdInBackground)
  # if we want to run in the background, just launch the cmd and return
  #---------
  if runCmdInBackground:
    os.system("%s &" % cmdtxt)
    return (0,"No output from background os.system(%s) call" % cmdtxt)
  # else this is a blocking call, use Popen
  #---------
  try:
    p = subprocess.Popen(cmdtxt, stdout=subprocess.PIPE, shell=True,stderr=subprocess.STDOUT)
  except KeyboardInterrupt:
#    print "...got ctrl-c..."
    pid = p.pid
    p.terminate()
    p.send_signal(signal.SIGKILL)
    # Check if the process has really terminated & force kill if not.
    try:
        os.kill(pid, signal.SIGKILL)
        p.kill()
        print "Forced kill"
    except OSError, e:
        print "Terminated gracefully"
  
  (output, err) = p.communicate()
  p_status = p.wait()
  outtxt = output.strip()
  print "# >>>> SUBPROCESS_INFO_START(%s) >>>>" % cmdtxt
  app = cmdtxt.split(' ', 1)[0]
  for ln in outtxt.splitlines():
    print " +%s+> %s" % (app,ln)
  print "# <<<< SUBPROCESS_INFO_END(%s) <<<< " % cmdtxt
  if p_status > 0:
    print "ERROR: error returned from following command,"
    print "ERROR:      $ ",cmdtxt
  print "# DBG:  SUBPROCESS END:%s retcode=%d" % (cmdtxt,p_status)
  return (p_status,outtxt)

#-----------------------------------
def run_popup_warning_window(warning_txt):
    txt = "mac_popup.sh \"%s\"" % warning_txt
    print "# WARNING:run_popup_warning_window: %s" % warning_txt
    (ret,res) = run_subprocess(txt)
    print "# WARNING: window returned ret=",ret," result txt=",res
    return (ret,res)

#-----------------------------------
def get_face_min_max_give_scaling(STL_FILE,PACK_FILE,scale,TUBE_RADIUS,TUBE_SCALE,avg_size): #scale,avg_size):
  fmin=fmax=0
  print "# get_face_min_max_give_scaling..."
  cmdtxt = 'run_voro_fill_stl.sh -i %s %s %f %f %f %f' % (STL_FILE,PACK_FILE,scale,TUBE_RADIUS,TUBE_SCALE,avg_size)
  print "# CMD:",cmdtxt
  (ret,wes) = run_subprocess(cmdtxt)
  print wes
  for line in wes.splitlines():
    # FACE min,max:0.227601248498845,0.608093498845267
    a = re.match('^# FACE min,max:(.*),(.*)',line)
    if a:
      fmin=float(a.group(1))
      fmax=float(a.group(2))
  return (fmin,fmax)

#-----------------------------------
def fix_and_correct_STL(STL_FILE):
    return run_subprocess('fix_stl.sh %s' % STL_FILE)

#-----------------------------------
def info_return_poly_count(STL_FILE):
    #$ stl_info misc_stl/new_thing.stl
    ## ...
    ## # Number of polys = 9272
    print "# info_return_poly_count..."
    (ret,res) = run_subprocess("stl_info %s | grep '^# Number of polys =' | awk '{print $6}'" % STL_FILE)
    return int(res)



###################
# Compute linear regression slope & intercept from passed in CSV file of datapoints
#
# slope = (N * sum(xy) - sum(x)*sum(y)) / (N * sum(x^2) - sum(x)^2)
# intercept = (sum(y) - b*sum(x)) / N
class xy_slope:
    def __init__(self,save_filename,history):
        self._clear()
        self.history = history
        self.saveFn = save_filename
        if os.path.exists(self.saveFn):
          os.remove(self.saveFn)
    
    #--------
    # PUBLIC
    #--------
    def new_sample(self,x,y):
        # just append values to CSV file, this file is then read to get slope/intercept
        with open(self.saveFn, 'a') as the_file:
          the_file.write('%0.12f,%0.12f\n' % (x,y))

    # pull 'history' lines from the 'saveFile', compute slope/intercept from only those
    def get_slope_intercept(self):
        self._clear()
        lines = self._tail(self.saveFn,self.history)
        for line in lines:
          a = re.match('(.*),(.*)',line)
          if a:
            xin = float(a.group(1))
            yin = float(a.group(2))
            self._add_sample(xin,yin)
        if self.N == 0:
            return (0,0)
        if (self.N*self.sum_x2 - self.sum_x*self.sum_x) == 0:
            return (0,0)
        slope = (self.N*self.sum_xy - self.sum_x*self.sum_y) / (self.N*self.sum_x2 - self.sum_x*self.sum_x)
        intercept = (self.sum_y - slope*self.sum_x) / self.N
        return (slope,intercept)

    # y = mx + b
    def get_y_value_given_x(self,x):
        (m,b) = self.get_slope_intercept()
        return (m * x) + b
    # x = (y - b)/m
    def get_x_value_given_y(self,y):
        (m,b) = self.get_slope_intercept()
        return (y - b) / m;
    
    #--------
    # PRIVATE
    #--------
    def _clear(self):
        self.N = self.sum_xy = self.sum_x = self.sum_y = self.sum_x2 = 0
    
    def _add_sample(self,x,y):
        self.N += 1
        self.sum_xy += x * y
        self.sum_x += x
        self.sum_y += y
        self.sum_x2 += x * x

    def _tail(self,fn,n):
        stdin,stdout = os.popen2('tail -n %d %s' % (n,fn))
        stdin.close()
        lines = stdout.readlines(); stdout.close()
        return lines

def get_tempfile(nm):
  stdin,stdout = os.popen2('mktemp /tmp/%s.XXXXXXXXX' % (nm))
  stdin.close()
  tmpfn = stdout.readlines(); stdout.close()
  return tmpfn[0].rstrip()
###################



#-----------------------------------
class Point:
  """ Create a new Point, at coordinates x, y, z """
  def __init__(self, x=0, y=0, z=0):
      """ Create a new point at x, y, z """
      self.x = x
      self.y = y
      self.z = z
  def distance_from_origin(self):
      """ Compute my distance from the origin """
      return ((self.x ** 2) + (self.y ** 2) + (self.z ** 2)) ** 0.5
  def to_string(self):
      return "({0}, {1}, {2})".format(self.x, self.y, self.z)
  def halfway(self, target):
     """ Return the halfway point between myself and the target """
     mx = (self.x + target.x)/2
     my = (self.y + target.y)/2
     mz = (self.z + target.z)/2
     return Point(mx, my, mz)
  def minus(self,target):
     mx = (self.x - target.x)
     my = (self.y - target.y)
     mz = (self.z - target.z)
     return Point(mx, my, mz)
  def angleBetweenVector(self,vB):
    vA = Point(self.x,self.y,self.z)
    fCrossX = vA.y * vB.z - vA.z * vB.y
    fCrossY = vA.z * vB.x - vA.x * vB.z
    fCrossZ = vA.x * vB.y - vA.y * vB.x
    fCross = math.sqrt(fCrossX * fCrossX + fCrossY * fCrossY + fCrossZ * fCrossZ)
    fDot = vA.x * vB.x + vA.y * vB.y + vA.z + vB.z
    return math.atan2(fCross, fDot)




