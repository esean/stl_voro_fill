-----------------------
README.txt
-----------------------


  *************************************************
  ** This codebase assumes y-axis is pointing up **
  *************************************************


This code takes an STL and generates that STL filled with voronoi,

	$ source stl_voro_fill/sw/sourceMe.sh
	$ vi build_cfg.ini
	$ RunMe_MakeProduct.py 1in_cyl.stl
    ...
    VORO_SCALING_FIT=5.2039
	$ RunMe_MakeProduct.py -p 1in_cyl.stl 5.2039 &> log
    $ ViewSTL [output STL]



=======================
Prerequisites
=======================
* Install these first,
    $ brew install cmake gnu-sed

* If 'gnu-sed' was installed as 'gnu-sed', you need to make a symlink so it can be called as 'gsed',
    $ sudo ln -s /usr/local/opt/gnu-sed/bin/sed /usr/local/bin/gsed

* Then run 'make.sh'
    - You will need to manually launch the XQuartz installer. You will then have to log out and log back
      in to activate those changes. Once logged in again, launch XQuartz so xterm windows can be opened
      by the running scripts

* Copy admesh library,
    $ sudo cp bin/.libs/libadmesh.1.dylib /usr/local/lib



=======================
Licenses:
=======================
	Voro++: modified BSD license, that makes it free for any purpose
    admesh: GPL-2.0 (is compiled and called as standalone app)
	VTK: BSD

Voro++ and VTK are linked together

