#!/bin/bash

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

[ -d build ] && rm -rf build
[ ! -d build ] && mkdir build
cd build
cmake .. && \
make 

