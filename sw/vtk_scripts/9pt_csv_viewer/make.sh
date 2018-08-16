#!/bin/bash

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

make_vtk_app.sh && \
cp build/9pt_csv_viewer.app/Contents/MacOS/9pt_csv_viewer $PCL_VORO_PROJDIR/sw/bin
