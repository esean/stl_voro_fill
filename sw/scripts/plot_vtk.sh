#!/bin/bash
_script_counter.sh $0 $@

if [ -z "$PCL_VORO_PROJDIR" ]; then
	mac_popup.sh "$0: Need to source sw/sourceMe.sh" 
	exit 1
fi

APP=TubesFromSplines

TUBE_R_DEF=1
TUBE_S_DEF=0
OUT_DEF=out.stl
SC_DEF=1

usage() {
	echo
	echo "$0 [ARGS] [input CSV file] {[tube diameter] [tube scaling] [output STL filename] [scaling]}"
	echo
	echo " where optional {{[ARGS]} are,"
	echo "	-i	just provide info about input file rendering and exit"
	echo "	-p 	high-performance render, for 3d-printing"
	echo "	-n 	don't plot the render, just create STL and exit"
	echo
	echo "Default params:"
	echo "	[tube diameter]	= $TUBE_R_DEF"
	echo "	[tube scaling]	= $TUBE_S_DEF"
	echo "	[output STL filename]	= $OUT_DEF"
	echo "	[scaling]	= $SC_DEF-{n}.stl (where 'n' is the index of STL outputs}"
	echo
}

mtxt=''
binfo=0
bDoPlot=1
while getopts ":hipn" opt; do
  case $opt in
    h)	usage; exit 0;;
    i)	mtxt="${mtxt}-i " ; binfo=1;;
    p)	mtxt="${mtxt}-p " ;;
    n)  bDoPlot=0;;
    \?) echo "Invalid option: -$OPTARG" >&2
     	exit 1 ;;
    :) echo "Option -$OPTARG requires an argument." >&2
     	exit 1 ;;
  esac
done
shift $((OPTIND-1))

if [ -z "$1" ]; then
	usage
	exit 1
fi
	
IN="$1"
DIA=${2:-$TUBE_R_DEF}
RAD_SCALE=${3:-$TUBE_S_DEF}
OUT=${4:-$PWD/$OUT_DEF}
SCALE=${5:-$SC_DEF}

echo "$APP $mtxt $IN $DIA $RAD_SCALE $OUT $SCALE" > ${OUT}.build.cfg 
$APP $mtxt $IN $DIA $RAD_SCALE $OUT $SCALE
# TODO: only allows one at a time, whereas vtk can plot them all at same time

[ $binfo -eq 1 ] && exit 0

# TODO: assumes 1 output STL...
fix_stl.sh out.stl-0.stl

[ $bDoPlot -eq 0 ] && exit 0

# bring up a viewer showing render
ViewSTL out.stl-0.stl &

