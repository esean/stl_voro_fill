// Irregular packing example code
//
// Author   : Chris H. Rycroft (LBL / UC Berkeley)
// Email    : chr@alum.mit.edu
// Date     : August 30th 2011

#include "voro++.hh"
using namespace voro;

double xy_resolution = 1.5;     // sets max x,y
double dodec_init = 2.0;

// Set up constants for the container geometry
const double x_min=-xy_resolution,x_max=xy_resolution;
const double y_min=-xy_resolution,y_max=xy_resolution;
const double z_min=-3,z_max=9;

// Golden ratio constants
const double Phi=0.5*(1+sqrt(5.0));
const double phi=0.5*(1-sqrt(5.0));

// Set the computational grid size
const int n_x=14,n_y=14,n_z=7;

// Set the number of particles that are going to be randomly
// introduced
const int particles=1;

double rnd() {return double(rand())/RAND_MAX;}


double length_base = 0.0;
double total_length = 1.0;      // how long is cylinder
//double mesh_density = 1.5;      // larger = more mesh inside cylinder

//const double z_min=length_base,z_max=(length_qqbase+total_length);





// Create a wall class that, whenever called, will replace the Voronoi cell
// with a prescribed shape, in this case a dodecahedron
class wall_initial_shape : public wall {
	public:
		wall_initial_shape() {

			// Create a dodecahedron
			v.init(-dodec_init,dodec_init,-dodec_init,dodec_init,-dodec_init,dodec_init);
			v.plane(0,Phi,1);v.plane(0,-Phi,1);v.plane(0,Phi,-1);
			v.plane(0,-Phi,-1);v.plane(1,0,Phi);v.plane(-1,0,Phi);
			v.plane(1,0,-Phi);v.plane(-1,0,-Phi);v.plane(Phi,1,0);
			v.plane(-Phi,1,0);v.plane(Phi,-1,0);v.plane(-Phi,-1,0);
		};
		bool point_inside(double x,double y,double z) {return true;}
		bool cut_cell(voronoicell &c,double x,double y,double z) {

			// Set the cell to be equal to the dodecahedron
			c=v;
			return true;
		}
		bool cut_cell(voronoicell_neighbor &c,double x,double y,double z) {

			// Set the cell to be equal to the dodecahedron
			c=v;
			return true;
		}
	private:
		voronoicell v;
};

int main() {

	// Create a container with the geometry given above. This is bigger
	// than the particle packing itself.
	container con(x_min,x_max,y_min,y_max,z_min,z_max,n_x,n_y,n_z,
			false,false,false,8);

	// Create the "initial shape" wall class and add it to the container
	wall_initial_shape(wis);
	con.add_wall(wis);

#if 1
	// Import the irregular particle packing
	con.import("pack_irregular");
//
#else
    
    // Randomly insert particles into the container, checking that they lie
    // inside the tetrahedron
    int i=0;
    while(i<particles) {
        int x,y,z;
        x=x_min+rnd()*(x_max-x_min);
        y=y_min+rnd()*(y_max-y_min);
        z=z_min+rnd()*(z_max-z_min);
        if (con.point_inside(x,y,z)) {
            con.put(i,x,y,z);i++;
        }
    }
#endif
    
//	// Save the particles and Voronoi cells in POV-Ray format
//	con.draw_particles_pov("irregular_p.pov");
//	con.draw_cells_pov("irregular_v.pov");

    con.draw_cells_gnuplot("cad_small.gnu");
	con.print_custom("%B","cad_small.csv");
}
