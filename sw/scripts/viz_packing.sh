#!/bin/bash
_script_counter.sh $0 $@

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

if [ -z $1 ]; then
	echo "USAGE: $0 {-i} [packing file]"
	echo "Allows a closer look at data contained in [packing file]"
	exit 1
fi
info=0
if [ "$1" == "-i" ]; then
	info=1; shift
fi

U=`mktemp /tmp/XXXXXXXXXXXXX`
cat $@ | awk '{print $2}' | sort -g > $U.2
cat $@ | awk '{print $3}' | sort -g > $U.3
cat $@ | awk '{print $4}' | sort -g > $U.4

echo -n "COL_cellId:"
cat $@ | awk '{print $1}' | minmax.pl
echo -n "COL_x:"
cat $U.2 | minmax.pl
echo -n "COL_y:"
cat $U.3 | minmax.pl
echo -n "COL_z:"
cat $U.4 | minmax.pl
[ $info -eq 1 ] && exit 0

cat $@ | awk '{print $2" "$3" "$4}' > points_inside.gnu
gnuplot points_inside.gp &

plot_all.sh $U.2 $U.3 $U.4 | plotit.pl -C "x:y:z"


