from http://www.vtk.org/Wiki/VTK/Examples/Cxx/Filtering/TransformPolyData

to compile

$ source sw/sourceMe.sh
$ mkdir build
$ cd build
$ cmake -DVTK_DIR:PATH=$VTK_INSTALL_DIR/build ..
$ make
$ cd ..
(mac)
$ ./build/translate_stl.app/Contents/MacOS/translate_stl outer.stl 0 1 0
(linux)
$ ./build/translate_stl outer.stl 0 1 0
