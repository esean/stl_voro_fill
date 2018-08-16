#!/bin/bash

die() { echo "ERROR:$@:$0 !"; exit 1; }
[ -z "$1" ] && die "USAGE:$0 [STL]"

echo "# DBG: admesh --write-binary-stl=$1 $1"
admesh --write-binary-stl=$1 $1
echo "# DBG: clean_stl $1"
clean_stl $1
echo "# DBG: mv $1-clean.stl $1"
mv $1-clean.stl $1
