from http://www.vtk.org/Wiki/VTK/Examples/Cxx/VisualizationAlgorithms/TubesFromSplines

to compile

$ source sw/sourceMe.sh
$ mkdir build
$ cd build
$ cmake -DVTK_DIR:PATH=$VTK_INSTALL_DIR/build ..
$ make
$ cp 9pt_csv_viewer/Contents/MacOS/9pt_csv_viewer $PCL_VORO_PROJDIR/sw/bin

