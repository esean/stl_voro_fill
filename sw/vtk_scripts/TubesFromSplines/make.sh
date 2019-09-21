#!/bin/bash

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

cp_build()      # $1=project name
{
	is_linux.sh &&
		cp build/$1 $PCL_VORO_PROJDIR/sw/bin ||
		cp build/$1.app/Contents/MacOS/$1 $PCL_VORO_PROJDIR/sw/bin
}

make_vtk_app.sh && \
cp_build TubesFromSplines 
