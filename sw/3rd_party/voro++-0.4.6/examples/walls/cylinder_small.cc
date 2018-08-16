// Cylindrical wall example code
#include <time.h>
#include "voro++.hh"
#include <cstdio>
using namespace voro;

//------
#define CFG_GENERATE_SPHERE 0
#define CFG_GENERATE_CYLINDER 0
#define CFG_GENERATE_CYLINDER_SPHERETOP 0
#define CFG_GENERATE_BOX 0
#define CFG_GEN_USR_CB 1

const double LENGTH = 0.7;
const double WIDTH = 0.7; //1.16;  // 1.16" gives full diameter 1.75"
const char* bubble_packing_fn = "pc.new";
//const char* bubble_packing_fn = "points_inside3.new";
//const char* bubble_packing_fn = "pack_cylinder";
//------

// length & width
const double height_multiplier = 2.5;
double total_length0 = LENGTH * height_multiplier;   // want 4"
const double width_multiplier = 1.2087912088;
double total_width0 = WIDTH * width_multiplier;   // want 1.42"
// CYL voronoi zoom:
const double voronoi_zoom = 1.0;
double total_length = total_length0 * voronoi_zoom;
double total_width = total_width0 * voronoi_zoom;
// Set up constants for the container geometry
double xy_resolution = total_width;    // sets max x,y
const double x_min=-xy_resolution,x_max=xy_resolution;
const double y_min=-xy_resolution,y_max=xy_resolution;
#if CFG_GENERATE_SPHERE
double length_base = -2.0;
#else
double length_base = total_width;
#endif
const double z_min=length_base-total_length,z_max=length_base;

// Set the computational grid size
const int n_x=17,n_y=17,n_z=25;

// prevent a warning
#if CFG_GEN_USR_CB
// return True if the point is inside, false if the point is outside.
static bool voro_cb_point_inside(double x, double y, double z) {
    printf("# main:: voro_cb_point_inside:%f %f %f\n",x,y,z);
    return false;
}
// return True if the cell still exists, false if the cell is deleted.
static bool voro_cb_cut_cell(double x, double y, double z,
                             double* dx, double* dy, double* dz, double* dq, voro::wall_user_callback::cell_info* ci) {
    printf("# main:: voro_cb_cut_cell:%f %f %f\n",x,y,z);
    *dx = x;
    *dy = y;
    *dz = z;
    *dq = x*x+y*y+z*z;
    return true;
}
#endif

int main() {
    
    if (total_width > xy_resolution)
        fprintf(stderr,"# WARNING: cylinder is not perfectly round\n");
    
	// Create a container with the geometry given above, and make it
	// non-periodic in each of the three coordinates. Allocate space for
	// eight particles within each computational block.
    printf("# VORO container size: xmin%f,xmax%f,ymin%f,ymax%f,zmin%f,zmax%f\n",x_min,x_max,y_min,y_max,z_min,z_max);
	container con(x_min,x_max,y_min,y_max,z_min,z_max,n_x,n_y,n_z,
			false,false,false,8);
    
	// Add a cylindrical wall to the container
#if CFG_GENERATE_SPHERE
    wall_sphere	wall(0,0,0,total_width);        // set length_base = -2
    con.add_wall(wall);
#elif CFG_GENERATE_CYLINDER
	wall_cylinder wall(0,0,0,0,0,1,total_width);
    con.add_wall(wall);
#elif CFG_GENERATE_CYLINDER_SPHERETOP
    wall_cylinder_sphere_end wall(0,0,0,0,0,1,total_width);
    con.add_wall(wall);
#elif CFG_GENERATE_BOX
    #warning "BOX: not making any walls..."
#elif CFG_GEN_USR_CB
    wall_user_callback wall(0,0,0,&voro_cb_cut_cell,&voro_cb_point_inside);
    con.add_wall(wall);
#else
#error "ERROR: define a CFG_GENERATE_xxx defins"
#endif
    
    con.import(bubble_packing_fn);
//	con.compute_all_cells();
//	double vol = con.sum_cell_volumes();
//	int tp=con.total_particles();
//	printf("volume = %f, particles = %d\n",vol,tp);

    con.draw_cells_gnuplot("cad_small.gnu");
	con.print_custom("%B","cad_small-cylinder.csv");
    
//    con.draw_particles();
}
