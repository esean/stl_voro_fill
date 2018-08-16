This code takes an STL and generates that STL filled with voronoi.

To get started, source this to set needed env.vars,

```
$ source stl_voro_fill/sw/sourceMe.sh
```

All scripts read the build.ini file for global build settings. You can adjust
the following to affect the build type,

    * set_face_mm: sets the avg. voronoi face size in mm. Setting larger
        makes each voronoi cell larger, so less cells are packed into an
        STL, but this could result in STL not being completely filled such
        that the design is not printable. Setting this smaller packs more
        cells into an STL, but if too small the cell tubes will collide
        with each other. In that case you could reduce TUBE_RADIUS

    * TUBE_RADIUS: sets radius of each cell tube element. The default is
        good for 3D printing. You can increase this value but would also
        need to increase set_face_mm 
            
```
$ vi build_cfg.ini
```

Now first the script needs to be run to find the scaling factor,

```
$ RunMe_MakeSTL.py 1in_cyl.stl
    ...
COL_y:COUNT:7464 MIN:-42.5519892261 MAX:42.5922032262 DELTA:85.1441924523 AVG:-0.0440457234595226 STDDEV:24.6162030419868 TOTAL:-328.757279901877
COL_z:COUNT:7464 MIN:-0.6647247013 MAX:84.2987207013 DELTA:84.9634454026 AVG:41.9552812464444 STDDEV:24.5898149894429 TOTAL:313154.219223461
RUN...
PROC...
# FACE min,max:1.28576454789458,3.30612026206079
# TOTAL splines = 73836, faces = 14344, cells = 1019
# x(-43.059200,43.066299) y(-33.350399,30.811800) z(0.000000,83.622704) max_dist = 47.766369
scale= 5.3359162341  min= 1.28576454789  max= 3.30612026206  avg= 2.29594240498
VORO_SCALING_FIT:5.335916
```

The value printed on the last line "VORO_SCALING_FIT" is the scaling factor. Now
call the script again with this number and the design will be rendered.

    !!    Depending on the size of the provided STL, and the number of voronoi cells being filled, this
    !!    could take a substantial amount of time, CPU time, and RAM... For example, the STL in
    !!    examples/bunny.stl took me about 3hrs to render, with TubesFromSplines running at 100%
    !!    and using almost 1.4GB RAM at the end.
    !!
    !!    It is recommended to close all other apps when rendering large STL files.

To render the final output STL, start the process with,

```
$ RunMe_MakeSTL.py -p 1in_cyl.stl 5.335916 &> log
```

The output STL is written to out.stl-0.stl. This code provides an STL viewer to inspect,

```
$ ViewSTL out.stl-0.stl
```

You can provide N number of STLs on the cmd line and they will all be displayed at once,

```
$ ViewSTL out.stl-0.stl 1in_cyl.stl {[other STL]...}
```



# Prerequisites

Install these first,
```
$ brew install cmake gnu-sed
```

If 'gnu-sed' was installed as 'gnu-sed', you need to make a symlink so it can be called as 'gsed',
```
$ sudo ln -s /usr/local/opt/gnu-sed/bin/sed /usr/local/bin/gsed
```

Then make
```
$ ./make.sh
```

You will need to manually launch the XQuartz installer. You will then have to log out and log back
in to activate those changes. Once logged in again, launch XQuartz so xterm windows can be opened
by the running scripts

Copy admesh library,
```
$ sudo cp bin/.libs/libadmesh.1.dylib /usr/local/lib
```



# Licenses:

    Voro++: modified BSD license, that makes it free for any purpose
    admesh: GPL-2.0
    VTK: BSD

Voro++ and VTK are linked together. Admesh is compiled and called as standalone app.


# Notes:

This codebase assumes y-axis is pointing up 


