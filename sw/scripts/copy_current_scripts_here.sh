#!/bin/bash

# for copying current scripts into dir sw/ in $PWD.
# used for archiving current settings

die() { echo "ERROR:$0:$@ !"; exit 1; }
[ -d sw ] && die "Dir sw/ already exists, remove it first"

mkdir sw
cp -a $PCL_VORO_PROJDIR/sw/scripts sw
cp -a $PCL_VORO_PROJDIR/sw/vtk_scripts sw

# remvoe any STLs copied over
find sw -name \*.stl -exec rm -f {} \;

