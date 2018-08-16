#!/bin/bash
_script_counter.sh $0 $@

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

if [ -z $1 ]; then
        echo "USAGE: $0 [.gnu file]"
	echo "Converts a x,y,z GnuPlot .gnu file to a [packing file]. This just"
	echo "means a cellId (or integer count) is prepended to each line."
        exit 1
fi

FN="$1"
if [ ! -f $FN ]; then
	echo "ERROR: Cannot find [.gnu file] $FN"
	exit 1
fi

cat $FN | nl | awk '{print $1" "$2" "$3" "$4}'

