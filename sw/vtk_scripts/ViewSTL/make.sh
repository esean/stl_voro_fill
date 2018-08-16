#!/bin/bash

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

make_vtk_app.sh && \
cp build/ViewSTL.app/Contents/MacOS/ViewSTL $PCL_VORO_PROJDIR/sw/bin
