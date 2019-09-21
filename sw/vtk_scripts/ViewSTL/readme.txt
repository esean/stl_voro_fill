from http://www.vtk.org/Wiki/VTK/Examples/Cxx/IO/ReadSTL

to compile

$ source sw/sourceMe.sh
$ mkdir build
$ cd build
$ cmake -DVTK_DIR:PATH=$VTK_INSTALL_DIR/build ..
$ make
(Mac)
$ cp ViewSTL.app/Contents/MacOS/ViewSTL $PCL_VORO_PROJDIR/sw/bin
(Linux)
$ cp ViewSTL $PCL_VORO_PROJDIR/sw/bin
$ cd ..
#$ ./build/ViewSTL cone.stl
