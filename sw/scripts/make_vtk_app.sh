#!/bin/bash
_script_counter.sh $0 $@

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

[ ! -d build ] && mkdir build
cd build
# '-G Xcode'
cmake -DVTK_DIR:PATH=$VTK_INSTALL_DIR/build .. && \
make
