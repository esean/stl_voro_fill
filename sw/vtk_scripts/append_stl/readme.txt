
to compile
----------

$ source sw/sourceMe.sh
$ mkdir build
$ cd build
$ cmake -DVTK_DIR:PATH=$VTK_INSTALL_DIR/build ..
$ make
(mac)
$ cp append_stl.app/Contents/MacOS/append_stl $PCL_VORO_PROJDIR/sw/bin
(linux)
$ cp append_stl $PCL_VORO_PROJDIR/sw/bin


