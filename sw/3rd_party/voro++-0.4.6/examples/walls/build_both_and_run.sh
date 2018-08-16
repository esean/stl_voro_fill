#!/bin/bash

high_perf='-p '
#high_perf=''

diameter=0.06
rad_scale=0.75
scale=10

rm -f cad_small-sphere.csv cad_small-frustrum.csv cad_small-cylinder.csv
rm -f out-sphere.stl out-frustrum.stl out-cylinder.stl
make 
if [ ! $? -eq 0 ]; then
	echo "ERROR: make failed"
	exit 1
fi

# IN=${1:-cad_small.csv}
# DIA=${2:-0.06}
# RAD_SCALE=${3:-0.75}
# OUT=${4:-$PWD/out.stl}
# SCALE=${5:-10.0}

#  ../plot_vtk.sh -p cad_small-sphere.csv 0.06 0.75

./sphere-small
../plot_vtk.sh $high_perf cad_small-sphere.csv $diameter $rad_scale $PWD/out-sphere.stl $scale

./frustum
###../plot_vtk.sh -p cad_small-frustrum.csv 0.1 0.75
../plot_vtk.sh $high_perf cad_small-frustrum.csv 1.0 $rad_scale $PWD/out-frustrum.stl $scale

./cylinder_small
../plot_vtk.sh $high_perf cad_small-cylinder.csv $diameter $rad_scale $PWD/out-cylinder.stl $scale

/Applications/Meshmixer.app/Contents/MacOS/meshmixer $PWD/out-cylinder.stl
#/Applications/Meshmixer.app/Contents/MacOS/meshmixer $PWD/out-frustrum.stl $PWD/out-cylinder.stl
