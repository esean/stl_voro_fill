#!/bin/bash

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

make_vtk_app.sh && \
cp build/voro_fill_stl.app/Contents/MacOS/voro_fill_stl $PCL_VORO_PROJDIR/sw/bin
