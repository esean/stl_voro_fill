#!/bin/bash

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

rm -rf $PCL_VORO_PROJDIR/sw/bin/.libs

./configure && \
make && \
cp -a .libs admesh $PCL_VORO_PROJDIR/sw/bin
