#!/bin/bash

die() { echo "ERROR:$0:$@ !"; exit 1; }
usage() {
	echo -e "\nUSAGE: $0 [Product original STL] {[other STL/file to save]...}\n"
	echo -e "Clears a directory leaving the original STL and config files needed to run RunMe_Product.py\n"
}

[ -z "$1" ] && die "`usage`"

mkdir u9
for fx in $@; do
	mv $fx u9
done
mv build_cfg.ini u9
mv model_cfg.ini u9
mv email_cred.ini u9
rm -f *	>& /dev/null # won't remove directory u9
mv u9/* .
rm -rf u9

