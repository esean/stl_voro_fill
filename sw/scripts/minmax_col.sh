#!/bin/bash
_script_counter.sh $0 $@

#-------------------------------------------------------------------
#
#  NOTE: any line starting with '##' will be shown for 'help'
#
## minmax_col.sh [-i] [-c [data column]] [-t [minmax.pl term]]
##
## Pipes data to minmax.pl. Allows for selecting which column
## to analyze (-c option) and which minmax.pl term (AVG,MIN,MAX,etc)
## with (-t option) to report back.
## Using [-i] opt will report the min/max avg values found from
## a dataset with two dominant values, ie, cping didt waveform.
##
#-------------------------------------------------------------------
debug=0

COL=1
term=''
minmax_opt=''

usage() { grep '^##' $0 | cut -d# -f3-; }
die() { echo -e "ERROR:$0:$@" >&2; exit 1; }
dlog() { [ $debug -eq 1 ] && echo "DEBUG:$0:$@"; }
while [ X"$1" != X ]; do
	case X"$1"X in
		X-hX|X--helpX)	usage; exit 0;;
		X-iX)	dlog "ARG: turning on didt mode"; minmax_opt="${minmax_opt} -i"; shift;;
		X-dX)	dlog "ARG: turning on debug"; debug=1; minmax_opt="${minmax_opt} -d"; shift;;
		X-cX)	dlog "ARG: setting column"; shift; COL="$1"; shift;;
		X-tX)	dlog "ARG: setting minmax term"; shift; term="$1"; shift;;
		X-*)	die "Unknown option: $1";;
		X[0-9a-zA-Z]*)	die "Unknown option: $1";;
	esac
done


#======================================================
# PASS: $1 = column to select, $2 = term key (MAX, MIN, AVG) from minmax.pl output, that value will be returned
# RETURNS: the requested value from minmax.pl
#
# FYI, the minmax.pl output looks like this,
#   COUNT:5000 MIN:0 MAX:124975000.0 DELTA:124975000 AVG:62487500 STDDEV:1.30208328125004e+15 TOTAL:312437500000
#======================================================
pull_minmax() {
	if [ X"$2" = X ]; then
		cut -d\  -f${1} | minmax.pl $minmax_opt 
	else
		cut -d\  -f${1} | minmax.pl $minmax_opt | tr ' ' '\n' | grep ^${2} | cut -d: -f2
	fi
}

pull_minmax $COL $term 
