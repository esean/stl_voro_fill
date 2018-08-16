#!/bin/bash

#
# THIS IS THE INSTALL SCRIPT FOR ALL SOFTWARE REQUIRED
#

if [ -z "$PCL_VORO_PROJDIR" ]; then
        mac_popup.sh "$0: Need to source sw/sourceMe.sh"
        exit 1
fi

type -p cmake
if [ ! $? -eq 0 ]; then
        mac_popup.sh "$0: Need to 'brew install cmake'"
        exit 1
fi

die() { echo -e "\nERROR:$0:$@ !"; exit 1; }

pushd $PCL_VORO_PROJDIR/sw

[ ! -d bin ] && mkdir bin

# install xquzrtz
open -W 3rd_party/XQuartz-2.7.11.dmg 

# Make required tools first
for dn in 3rd_party/admesh 3rd_party/voro++-0.4.6 3rd_party/VTK-7.1.0; do

	pushd $dn
	echo "# DBG: 3RD_PARTY: Building $dn/make.sh..."
	./make.sh || die "Failed building 3RD_PARTY: $dn"
	echo "# DBG: 3RD_PARTY: $dn done"
	popd &>/dev/null

done

# Make bin/ apps 
pushd vtk_scripts
./make_all.sh clean || die "Failed running make_all.sh"
popd &>/dev/null

