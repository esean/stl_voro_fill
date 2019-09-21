This code should run on both MacOS and Linux

RPI UPDATE:
--------------
* sudo apt update
* sudo apt install autotools-dev automake autoconf bc
  # from https://blog.kitware.com/raspberry-pi-likes-vtk/
* sudo apt install libgl1-mesa-dev libxt-dev libosmesa-dev
  # not required but useful: apt install meshlab


To install the software, run 

	$ ./make.sh



Problems:
* If you get this message, you need to change admesh configure script for the version of aclocal you have,

    config.status: creating config.h
    config.status: config.h is unchanged
    config.status: executing depfiles commands
    config.status: executing libtool commands
    configure:
    ==============================================================================
    Build configuration:
        werror:	  no
    ==============================================================================

    CDPATH="${ZSH_VERSION+.}:" && cd . && /bin/bash /home/seanharre/Documents/my_github/stl_voro_fill/sw/3rd_party/admesh/missing aclocal-1.16 -I m4
    /home/seanharre/Documents/my_github/stl_voro_fill/sw/3rd_party/admesh/missing: line 81: aclocal-1.16: command not found
    WARNING: 'aclocal-1.16' is missing on your system.
             You should only need it if you modified 'acinclude.m4' or
             'configure.ac' or m4 files included by 'configure.ac'.
             The 'aclocal' program is part of the GNU Automake package:
             <http://www.gnu.org/software/automake>
             It also requires GNU Autoconf, GNU m4 and Perl in order to run:
             <http://www.gnu.org/software/autoconf>
             <http://www.gnu.org/software/m4/>
             <http://www.perl.org/>
    Makefile:434: recipe for target 'aclocal.m4' failed
    make: *** [aclocal.m4] Error 127

    ERROR:./make.sh:Failed building 3RD_PARTY: 3rd_party/admesh !



    * Which version you have?
        $ aclocal-<TAB><TAB>
    * Edit file to use that version
        stl_voro_fill/sw$ vi 3rd_party/admesh/configure
    * Change this value to yours, here if I have aclocal-1.15
        am__api_version='1.15'


