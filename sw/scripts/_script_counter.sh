#!/bin/bash
#--------------------------------------------------------------------------------------------
# _script_counter.sh
#
# This script simply keeps track of which scripts are being executed.
#
# To enable it, it must be added at the top of each script that will be tracked like,
#
# Bash:
#	_script_counter.sh $0 $@
# Ruby:
#	`_script_counter.sh #{$0} #{ARGV.join(' ')}`
# Perl:
#	`_script_counter.sh $0 @ARGV`;
# C:
#        char XX1[1024],XX2[1024]; int XXd=argc; sprintf(XX1,"%s ",argv[0]);
#        while (--XXd) sprintf(XX1,"%s%s ",XX1,argv[argc-XXd]); XX1[strlen(XX1)-1] = '\0';
#        sprintf(XX2,"_script_counter.sh %s",XX1); system(XX2);
# Python:
#	import sys
#	import subprocess
#	subprocess.call("_script_counter.sh %s" % " ".join(sys.argv), shell=True)
#
# TODO:
#  * currently there is no logfile locking, so could get clobbered by
#    multiple processes writting at same time...
#--------------------------------------------------------------------------------------------
LOG="$PCL_VORO_PROJDIR/sw/var/_script_counter.log"

cmd=$1; shift; args="$@"
if [ X"$cmd" != X ]; then
	ok=1
	case X"$cmd"X in
		X-hX|X--helpX)	echo "$0 [command and args that are being run]. More help:"; grep ^# $0; ok=0;;
		XbashX|X-bashX) ok=0;;	# don't want to exit here just in case we are called from script being sourced
	esac
	if [ $ok -eq 1 ]; then
		dLOG=$(dirname $LOG)
		[ ! -d $dLOG ] && ok=0	# !ok if dir doesn't exist
		if [ $ok -eq 1 ]; then
			if [ ! -f $LOG ]; then
				touch $LOG
				chmod a+w $LOG
			fi
			[ ! -f $LOG ] && ok=0	# we must not have write access here, oh well...
			if [ $ok -eq 1 ]; then
				stime="`date +%s`"		# computer-readable
				utime="`date +%F-%H%M%S`"	# human-readable
				thisPWD="`/bin/pwd`"
				host="`uname -n`"
				hostver="`uname -r`"
				echo "$stime:$utime:$USER:$host:$hostver:$TB_CP_TARGET:$thisPWD:$cmd:$args" >> $LOG
			fi
		fi
	fi
fi
