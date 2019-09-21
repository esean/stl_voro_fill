from http://www.vtk.org/Wiki/VTK/Examples/Cxx/VisualizationAlgorithms/TubesFromSplines

to compile

$ source sw/sourceMe.sh
$ mkdir build
$ cd build
$ cmake -DVTK_DIR:PATH=$VTK_INSTALL_DIR/build ..
$ make
(mac)
$ cp TubesFromSplines.app/Contents/MacOS/TubesFromSplines $PCL_VORO_PROJDIR/sw/bin
(linux)
$ cp TubesFromSplines $PCL_VORO_PROJDIR/sw/bin

