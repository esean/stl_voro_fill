# Voro++ makefile
#
# Author : Chris H. Rycroft (LBL / UC Berkeley)
# Email  : chr@alum.mit.edu
# Date   : August 30th 2011

# Load the common configuration file
include ../../config.mk

# List of executables
#EXECUTABLES=cylinder
EXECUTABLES=cylinder_small
#EXECUTABLES=frustum cylinder_small sphere_small
#EXECUTABLES=cylinder tetrahedron frustum torus cylinder_small sphere_small

# Makefile rules
all: $(EXECUTABLES)

sphere_small: sphere_small.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o sphere_small sphere_small.cc -lvoro++

cylinder_small: cylinder_small.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o cylinder_small cylinder_small.cc -lvoro++

cylinder: cylinder.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o cylinder cylinder.cc -lvoro++

tetrahedron: tetrahedron.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o tetrahedron tetrahedron.cc -lvoro++

frustum: frustum.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o frustum frustum.cc -lvoro++

torus: torus.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o torus torus.cc -lvoro++

clean:
	rm -f $(EXECUTABLES)

.PHONY: all clean
