// Cylindrical wall example code

#include <time.h>
#include "voro++.hh"
#include <cstdio>
using namespace voro;

double total_length = 4;        // how long is cylinder, 6.0=2.4626", 0.426 per in
double mesh_density = 1.1;      // larger = more mesh inside cylinder, 1.1=0.91"

double xy_resolution = mesh_density;    // sets max x,y

// Set up constants for the container geometry
const double x_min=-xy_resolution,x_max=xy_resolution;
const double y_min=-xy_resolution,y_max=xy_resolution;
double length_base = -2;
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
const int n_x=17,n_y=17,n_z=25;

// This function returns a random double between 0 and 1
double rnd() {return double(rand())/RAND_MAX;}



int main() {
    
    srand ( time(NULL) );
    
    if (mesh_density > xy_resolution)
        fprintf(stderr,"# WARNING: cylinder is not perfectly round\n");
    
	// Create a container with the geometry given above, and make it
	// non-periodic in each of the three coordinates. Allocate space for
	// eight particles within each computational block.
	container con(x_min,x_max,y_min,y_max,z_min,z_max,n_x,n_y,n_z,
			false,false,false,8);
    
	// Add a cylindrical wall to the container
//	wall_cylinder cyl(0,0,0,0,0,1,mesh_density);
    wall_sphere	cyl(0,0,0,mesh_density);        // set length_base = -2
	con.add_wall(cyl);
    
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

    con.draw_cells_gnuplot("cad_small.gnu");
	con.print_custom("%B","cad_small-sphere.csv");
}
