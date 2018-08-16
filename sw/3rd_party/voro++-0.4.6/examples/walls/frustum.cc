// Frustum example code
//
// Author   : Chris H. Rycroft (LBL / UC Berkeley)
// Email    : chr@alum.mit.edu
// Date     : August 30th 2011
#include <time.h>
#include "voro++.hh"
using namespace voro;

const double pi=3.1415926535897932384626433832795;

double xy_resolution = 1.25;     // sets max x,y
double length_base = 1.0;
double total_length = 1.0;      // how long is cylinder

// Set up constants for the container geometry
const double x_min=-xy_resolution,x_max=xy_resolution;
const double y_min=-xy_resolution,y_max=xy_resolution;
const double z_min=length_base,z_max=(length_base+total_length);

const double dx = x_max - x_min;
const double dy = y_max - y_min;
const double dz = z_max - z_min;
const double divsx = 4.0;
const double divsy = 4.0;
const double divsz = total_length * 2;
const double scalingx = 0.65;
const double scalingy = 0.65;
const double scalingz = 0.85;

// Set the computational grid size
const int n_x=14,n_y=14,n_z=14;

// Set the number of particles that are going to be randomly
// introduced
//const int particles=45;

double rnd() {return double(rand())/RAND_MAX;}

int main() {
    
    srand ( time(NULL) );
    
	double evol,vvol;

	// Create a container with the geometry given above, and make it
	// non-periodic in each of the three coordinates. Allocate space for
	// eight particles within each computational block.
    //x_min,x_max,y_min,y_max,z_min,z_max,, //
	container con(x_min,x_max,y_min,y_max,z_min,z_max,n_x,n_y,n_z,
			false,false,false,8);

	// Add a cylindrical wall to the container
    wall_cone cone(0,0,3,0,0,-1,0.55); //tan(0.65));
	con.add_wall(cone);

    
    // Place particles in a regular grid within the frustum, for points
    // which are within the wall boundaries
    int i=0;
    int flip = 0;
    double x_min2 = x_min;
    double y_min2 = y_min;
    for(double z=z_min+dz/divsz/2;z<=z_max;z+=dz/divsz) {
        x_min2 = x_min;
        y_min2 = y_min;
        if (flip) {
            x_min2 += dx/divsx/2;
            y_min2 += dy/divsy/2;
        } else {
            x_min2 -= dx/divsx/2;
            y_min2 -= dy/divsy/2;
        }
        flip = !flip;
        for(double y=y_min2;y<=y_max;y+=dy/divsy)
            for(double x=x_min2;x<=x_max;x+=dx/divsx) {
            double xv,yv,zv;
            xv = x + (dx/divsx) * scalingx * (rnd()-0.5);
            yv = y + (dy/divsy) * scalingy * (rnd()-0.5);
            zv = z + (dz/divsz) * scalingz * (rnd()-0.5);
            if (con.point_inside(xv,yv,zv)) {
                //                    printf("# POINT %d: (%f,%f,%f)\n",i,xv,yv,zv);
                con.put(i,xv,yv,zv);i++;
            }
        }
    }
    
    
	// Compute the volume of the Voronoi cells and compare it to the
	// exact frustum volume
	evol=pi*1*(0.5*0.5+0.5*1+1*1)/3;
	vvol=con.sum_cell_volumes();
	printf("Exact frustum volume : %g\n"
	       "Voronoi cell volume  : %g\n"
	       "Difference           : %g\n",evol,vvol,vvol-evol);

    con.draw_cells_gnuplot("cad_small.gnu");
    con.print_custom("%B","cad_small-frustrum.csv");
}
