#!/bin/bash
_script_counter.sh $0 $@

#
# Given an STL file, find those Voronoi cells inside that shape and output a 3d voronoi tesselation as STL
#

TUBE_RAD_DEF=1
TUBE_SCALE_DEF=0	# 0=no scaling
#TUBE_SCALE_DEF=0.1
CELL_FACE_DEF=4		# mm

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi
die() { echo -e "\nERROR:$0:$@ !"; exit 1; }

usage() {
	echo 
	echo "USAGE: $0 {[ARGS]} [STL file] [filename of packing-file \"cube\" at (0,0,0)] [packing ratio] {tube [radius] [scaling] [cell-face mm]}"
	echo
	echo "Where optional {[ARGS]} can be,"
	echo "	-i	just display info and exit"
	echo "	-p	render in high-performance, suitable for 3d-printing"
	echo
	echo "Given an STL file, find those Voronoi cells inside that shape and output a 3d voronoi tesselation as STL"
	echo 
	echo "{tube [radius] [scaling]} defaults:"
	echo "	{[tube radius]}		$TUBE_RAD_DEF"
	echo "	{[tube scaling]}	$TUBE_SCALE_DEF"
	echo "	{[cell-face mm]}	$CELL_FACE_DEF"
}

if [ -z "$3" ]; then
	usage
	exit 1
fi

binfo=0
RENDER_PERF=''		# general-purpose debug render
while getopts ":hip" opt; do
  case $opt in
    h)      usage; exit 0;;
    i)  binfo=1;;
    p)	RENDER_PERF='-p';;	# high-performance render, for 3d printing
    \?) echo "Invalid option: -$OPTARG" >&2
        exit 1 ;;
    :) echo "Option -$OPTARG requires an argument." >&2
        exit 1 ;;
  esac
done
shift $((OPTIND-1))

STL_IN="$1"; shift
PACK_INI="$1"; shift
if [ ! -f $PACK_INI ]; then
	echo "ERROR: cannot find [packing file] $PACK_INI"
	exit 1
fi
PR="$1"; shift
tube_radius=$TUBE_RAD_DEF
if [ ! -z "$1" ]; then
	tube_radius=$1
	shift
fi
tube_scaling=$TUBE_SCALE_DEF
if [ ! -z "$1" ]; then
	tube_scaling=$1
	shift
fi
cellface_mm=$CELL_FACE_DEF
if [ ! -z "$1" ]; then
	cellface_mm=$1
	shift
fi

# fix any errors with STL
# NOTE: already done in RunMe_MakeSTL.py
#fix_stl.sh $STL_IN

# generate [packing file] by scaling [PACK_INI]
PACK_FN='pc.new'	# this is what's passed to voro_fill_stl
PACK_BASE="pack-cube-$PR"
echo "PACK_INI:"
viz_packing.sh -i $PACK_INI
xlate_pack_cylinder.py $PACK_INI 1 0 0 0 $PR $PR $PR 0 > $PACK_BASE
echo "PACK_BASE:"
viz_packing.sh -i $PACK_BASE

# build, and copies app to sw/bin/
rm -f cad_small-cylinder.csv cad_small.gnu $PACK_FN

# show STL size
U=`mktemp /tmp/XXXXXXXXXX`
echo "STL SIZE INFO..."
echo "voro_fill_stl -i $STL_IN $PACK_FN $cellface_mm"
voro_fill_stl -i $STL_IN $PACK_FN $cellface_mm
if [ ! $? -eq 0 ]; then
	echo "ERROR: error getting info about $STL_IN. Check the STL is solid and doesn't have any errors. Use MeshMixer -> analysis -> fix all errors"
	exit 1
fi
echo "voro_fill_stl -i $STL_IN $PACK_FN $cellface_mm"
voro_fill_stl -i $STL_IN $PACK_FN $cellface_mm > $U.3

# and make a packing file to fill that space, voro_fill_stl uses this '$PACK_FN' file as packing file input
echo "STL packing file creation..."
cat $U.3 | grep '# MODEL: bounds:' | cut -d: -f3 > $U
# MODEL: center = (-0.085287,-0.240374,29.737497)
cat $U.3 | grep '# MODEL: center = ' | cut -d\( -f2 | tr -d ')' | tr ',' ' ' > $U.center
# CENTEROFMASS:
cat $U.3 | grep '# MODEL: centerOfMass = ' | cut -d\( -f2 | tr -d ')' | tr ',' ' ' > $U.centerMass
echo "make_cube_this_big.sh $PACK_BASE `cat $U` `cat $U.center` > $PACK_FN"
make_cube_this_big.sh $PACK_BASE `cat $U` `cat $U.center` > $PACK_FN
viz_packing.sh -i $PACK_FN
rm -f $U $U.3 $PACK_BASE

# now run the tool that fills that STL space with voro
echo "RUN..."
voro_fill_stl $STL_IN $PACK_FN $cellface_mm 2>&1 | tee log_run.log | grep inside > $U.points
rm -f $PACK_FN log_run.log
[ ! -s cad_small-cylinder.csv ] && die "No points were found in that shape, maybe it is too small??"

# visualize results
echo "PROC..."
# -f3 are scaled 'cad' points, -f4 are non-scaled 'voro' points
# VORO: voro_cb_point_inside:1.448460 -0.615845 -19.631043:1.140519 -0.484917 -7.728757: inside? 0
cat $U.points | grep 1$ | cut -d: -f4 > points_inside.gnu
#cat $U.points | grep 0$ | cut -d: -f4 > points_outside.gnu

# check face min/max sizes
plot_vtk.sh -i cad_small-cylinder.csv > $U.4
cat $U.4 | grep 'DELTAS' | cut -d: -f2- | cut -d\  -f2- > face_data.csv
fmin=`cat face_data.csv | fpget_csv_column.rb -n face_min | minmax_col.sh -t AVG`
fmax=`cat face_data.csv | fpget_csv_column.rb -n face_max | minmax_col.sh -t AVG`
echo "# FACE min,max:$fmin,$fmax"
rm -f face_data.csv

# show face, splines info
cat $U.4 | grep '^# TOTAL splines ='
cat $U.4 | grep '^# x('
rm -f $U.4


#---------
[ "$binfo" -eq 1 ] && exit
# PLOT
#---------


#gnuplot points_inside.gp &
#gnuplot points_outside.gp &
#gnuplot x11_plot.gp &

# start STL and check face-size, want to repeat above until size is X (TBD)
# high-performance / printing quality & fix any problems with STL with admesh
# '-p' param to plot_vtk.sh make high-perf
cat <<EOM > $U.cmd
rm -f out.stl-0.stl
plot_vtk.sh -n $RENDER_PERF cad_small-cylinder.csv $tube_radius $tube_scaling 2>&1 | tee plot_vtk.log
echo; echo
echo "#------------ ERRORS -------------"
cat plot_vtk.log | grep ERROR
echo "#------------ ERRORS -------------"
echo; echo
EOM

xterm -e "`cat $U.cmd`" &


