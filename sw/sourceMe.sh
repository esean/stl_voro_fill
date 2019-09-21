#!/bin/bash
called=$_
#[[ $called != $0 ]] && echo "Script is being sourced" || echo "Script is being run"
sourceMesh_dir="`dirname ${BASH_SOURCE[0]}`"
#----------

export PCL_VORO_PROJDIR="$PWD/`dirname $sourceMesh_dir`"
export VTK_INSTALL_DIR="$PCL_VORO_PROJDIR/sw/3rd_party/VTK-7.1.0"

export PATH=$PCL_VORO_PROJDIR/sw/scripts:$PCL_VORO_PROJDIR/sw/bin:$PATH

[ ! -d $PCL_VORO_PROJDIR/sw/var ] && mkdir -p $PCL_VORO_PROJDIR/sw/var
