#!/bin/bash
_script_counter.sh $0 $@

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

if [ -z $7 ]; then
        echo "USAGE: $0 [packing file] [maxX] [maxY] [maxZ] [centerX] [centerY] [centerZ]"
	echo "Takes a cell-packing file and uses it to generate a space of maxX,maxY,maxZ, centered at (center{XYZ})"
	echo "by copying-pasting the [packing file] at multiple integer locations."
	echo
	echo "A [packing file] looks like,"
	echo -e "1 -0.382406 0.041601 -8.472787\n2 1.025165 -0.301089 -8.160780\n3 2.243533 0.147560 -8.010337"
	echo
	echo "A [packing file] can be visualized with,"
	echo "	viz_packing.sh pc.new"
        exit 1
fi

mk_float() { awk '{printf("%0.12f",$1)}'; }
mk_decimal() { awk '{printf("%d\n",$1)}'; }

inf=$1
dimx=$2
dimy=$3
dimz=$4
cenx=$5
ceny=$6
cenz=$7

U=`mktemp /tmp/XXXXXXXXXX`
viz_packing.sh -i $inf > $U

# COL_cellId:COUNT:622 MIN:604 MAX:1714 DELTA:1110 AVG:1151.52411575563 STDDEV:91329.278357337 TOTAL:716248
# COL_x:COUNT:622 MIN:0.0 MAX:4.78946008296 DELTA:4.78946008296 AVG:2.41151891896244 STDDEV:1.91580677948069 TOTAL:1499.96476759464
# COL_y:COUNT:622 MIN:0.0 MAX:4.78704272544 DELTA:4.78704272544 AVG:2.38630767456154 STDDEV:1.93278243329797 TOTAL:1484.28337357728
# COL_z:COUNT:622 MIN:0.0 MAX:4.77688056588 DELTA:4.77688056588 AVG:2.40398961840135 STDDEV:1.94066206373838 TOTAL:1495.28154264564

xrange=`cat $U | grep ^COL_x | sed 's/^.*DELTA:\(.*\).*AVG:.*$/\1/'`
yrange=`cat $U | grep ^COL_y | sed 's/^.*DELTA:\(.*\).*AVG:.*$/\1/'`
zrange=`cat $U | grep ^COL_z | sed 's/^.*DELTA:\(.*\).*AVG:.*$/\1/'`

# compute how many blocks fit in here, round up
xblocks="`echo "($dimx/$xrange)+1" | bc -l | mk_decimal`"
yblocks="`echo "($dimy/$yrange)+1" | bc -l | mk_decimal`"
zblocks="`echo "($dimz/$zrange)+1" | bc -l | mk_decimal`"

#echo "# Total size ($dimx,$dimy,$dimz) requires ($xblocks,$yblocks,$zblocks)"
rm -f $U.2
xcnt=0;ycnt=0;zcnt=0
while [ $xcnt -lt $xblocks ]; do
	while [ $ycnt -lt $yblocks ]; do
		while [ $zcnt -lt $zblocks ]; do

			place_pack_cube_at_xyz.py $inf $xcnt $ycnt $zcnt >> $U.2
			zcnt=$((zcnt+1))
		done
		zcnt=0
		ycnt=$((ycnt+1))
	done
	ycnt=0
	xcnt=$((xcnt+1))
done

# now center the entire cube at cen{xyz}
viz_packing.sh -i $U.2 > $U

# TODO: optimize
xrange=`cat $U | grep ^COL_x | sed 's/^.*DELTA:\(.*\).*AVG:.*$/\1/'`
yrange=`cat $U | grep ^COL_y | sed 's/^.*DELTA:\(.*\).*AVG:.*$/\1/'`
zrange=`cat $U | grep ^COL_z | sed 's/^.*DELTA:\(.*\).*AVG:.*$/\1/'`
xmin=`cat $U | grep ^COL_x | sed 's/^.*MIN:\(.*\).*MAX:.*$/\1/'`
ymin=`cat $U | grep ^COL_y | sed 's/^.*MIN:\(.*\).*MAX:.*$/\1/'`
zmin=`cat $U | grep ^COL_z | sed 's/^.*MIN:\(.*\).*MAX:.*$/\1/'`
# compute offset to center
xoff=`echo "-1.0*($xrange/2)-($xmin)+$cenx" | bc -l | mk_float`
yoff=`echo "-1.0*($yrange/2)-($ymin)+$ceny" | bc -l | mk_float`
zoff=`echo "-1.0*($zrange/2)-($zmin)+$cenz" | bc -l | mk_float`

#echo "# DBG:make_cube_this_big.sh: cenxyz = $cenx $ceny $cenz" >> pl
#echo "# DBG:make_cube_this_big.sh:xlate_pack_cylinder.py $U.2 1 $xoff $yoff $zoff 1 1 1 0" >> pl

xlate_pack_cylinder.py $U.2 1 $xoff $yoff $zoff 1 1 1 0


