#!/bin/bash
die() { echo "$0:$@ !"; exit 1; }
for i in *; do
	if [ -f $i/make.sh ]; then
		pushd $i
		[ "$1" == "clean" ] && rm -rf build
		./make.sh || die "Could not make $i"
		popd
	fi
done
