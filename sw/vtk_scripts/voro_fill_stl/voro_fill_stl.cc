// Combined VTK & Voro++ source codes
// 2-11-17 - mess created, can just fill a pre-defined shape (sphere, cylinder, etc)
// 2-12-17 - pass in STL, voro will fill inside that
// 3-8-17  - added 2nd slightly expanded shell for better point-inside detection (later removed)
// 4-9-17  - adding avg.neighbor inner.normal distance calc for cell center rejection (TODO: add tubesfromsplines, pipe diameter)
//

// flow is:
//  1. user passes in input STL, and packing-cube ("cell centers" x,y,z file)
//  2. constrcut voro wall using PointInsideSTL(x,y,z) & CutCell(x,y,z) callbacks
//  3. for each cell center, cut plane for is_insider(STL) (we need to play...)
//   3b. some cell centers rejected based on density per avg.neighbor-normal-distance-mm/min-voro-cell-mm

#include "../common/vtk_common.h"

#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkCellLocator.h>
#include <vtkCellData.h>
#include "voro++.hh"
#include <cstdio>
using namespace voro;

#define VTK_CREATE(type, name)  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//float MIN_CHANNEL_MM = -1.0;     // can't reduce it any further past this channel (+ 2x [extrude in mm])
float TUBE_RADIUS = -1.0;
//------
#define CFG_GENERATE_SPHERE 0
#define CFG_GENERATE_CYLINDER 0
#define CFG_GENERATE_CYLINDER_SPHERETOP 0
#define CFG_GENERATE_BOX 0
#define CFG_GENERATE_WALL_FROM_STL 1

const double LENGTH = 1.0;
const double WIDTH = 1.16; // 1.16" gives full diameter 1.75"
// length & width
const double height_multiplier = 2.54;
double total_length0 = LENGTH * height_multiplier;
const double width_multiplier = 2.54/2.0;
double total_width0 = WIDTH * width_multiplier;
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


void config_enclosedPoints_inner(double scale);
bool GetCellNormals(vtkPolyData* polydata, vtkIdType cellId, double* cN);

float Gcell_face_mm;

// globals...
vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
vtkSmartPointer<vtkCellLocator> cellLocator = vtkSmartPointer<vtkCellLocator>::New();
vtkSmartPointer<vtkSelectEnclosedPoints> selectEnclosedPoints = vtkSmartPointer<vtkSelectEnclosedPoints>::New();
vtkSmartPointer<vtkPolyData> stl_inner_shell = vtkSmartPointer<vtkPolyData>::New();
model_info info;	// TODO: maybe extrude_polydata_along_normal() can calc this itself, so don't need to pass to it...

/////////////////////////////////
////
//// Voro++ wall callbacks
////
/////////////////////////////////

// return True if the point is inside STL model, false if the point is outside.
static bool voro_cb_point_inside(double x, double y, double z) {
    
    double pt[3] = {x,y,z};
    bool is_inside = selectEnclosedPoints->IsInsideSurface(pt);
    bool is_inside_ex = false;
    printf("# VORO: voro_cb_point_inside:%0.8f %0.8f %0.8f:%0.8f %0.8f %0.8f: inside? ex%d:%d\n",pt[0],pt[1],pt[2],x,y,z,is_inside_ex,is_inside);

    // 031117: testing with slightly larger outside was not what wanted, voro needs to fit in given shape, so just look at those points
    return is_inside;// || is_inside_ex;
}

// return True if the cell still exists, false if the cell is deleted.
static bool voro_cb_cut_cell(double x, double y, double z,
                             double* dx, double* dy, double* dz, double* dq, voro::wall_user_callback::cell_info* ci) {
    
    // TODO: filter out some points when neighbor normals distance is increased
    if (!voro_cb_point_inside(x,y,z)) return false;

    double pt[3] = {x,y,z};
    //Find the closest points to TestPoint 'pt'
    double closestPoint[3];     //the coordinates of the closest point will be returned here
    double closestPointDist2;   //the squared distance to the closest point will be returned here
    vtkIdType cellId;           //the cell id of the cell containing the closest point will be returned here
    int subId;                  //this is rarely used (in triangle strips only, I believe)
    cellLocator->FindClosestPoint(pt, closestPoint, cellId, subId, closestPointDist2);
//    printf("# VORO: voro_cb_cut_cell(%f,%f,%f) centroid(%f,%f,%f)\n",pt[0],pt[1],pt[2],center[0],center[1],center[2]);
//    std::cout << "# VTK Coordinates of closest point: " << closestPoint[0] << " " << closestPoint[1] << " " << closestPoint[2] << std::endl;
//    std::cout << "# VTK Squared distance to closest point: " << closestPointDist2 << std::endl;
//    std::cout << "# VTK CellId: " << cellId << " subId: " << subId << std::endl;
    
    double dq_ret = closestPointDist2;
    // if too close to edge, say it's outside surface, one cell face_mm seems to work well, if you use Gcell_face/2.0, parts start sticking off
    float Gcell_face_mm2 = Gcell_face_mm * Gcell_face_mm;
    if (closestPointDist2 <= Gcell_face_mm2) {
        // todo: pull (x,y,z) back into surface by the amount (Gcell_face_mm - closestPointDist2)
        
//        return false;
        dq_ret = Gcell_face_mm2 - (Gcell_face_mm2 - closestPointDist2);
        //printf("# VORO:voro_cb_cut_cell(%f,%f,%f) is close to surface (%f mm^2), set *dq=%f\n",x,y,z,closestPointDist2,dq_ret);
        
    }
    
    // Try to read normals
    double cN[3];
    if( ! GetCellNormals(stl_inner_shell,cellId,cN))
    {
        std::cout << "ERROR: No cell normals were found" << std::endl;
        return false;
    }
//    printf("# VTK NORMAL:%f,%f,%f\n",cN[0],cN[1],cN[2]);
//    printf("# VORO:radius^2:%f faces:%d edges:%d edge_dist:%f surface_area:%f\n",
//           ci->max_radius_squared,ci->number_of_faces,ci->number_of_edges,ci->total_edge_distance,ci->surface_area);
//    //    printf("# main:: voro_cb_cut_cell:%f,%f,%f: inside? 1\n",pt[0],pt[1],pt[2]);
    *dx = cN[0];
    *dy = cN[1];
    *dz = cN[2];
    *dq = dq_ret;
//    printf("# VORODIST:%f = %f\n",closestPointDist2,dq_ret);
    return true;
}
/////////////////////////////////








void usage(char* str)
{
    shared_print_version();
    printf("USAGE: %s {[-i]} [input-STL filename] [pack-cube] [avg cell-face mm]\n",str);
}

int main(int argc, char *argv[])
{
    std::string rc;
//    if (read_config_file_key("MIN_CHANNEL_MM",&rc)) {
//        printf("ERROR: %s::read_config_file_key(MIN_CHANNEL_MM) returned error\n",argv[0]);
//        return EXIT_FAILURE;
//    }
//    MIN_CHANNEL_MM = (float)atof(rc.c_str());
//    printf("# DBG:%s:MIN_CHANNEL_MM=%f\n",argv[0],MIN_CHANNEL_MM);
    #
    if (read_config_file_key("TUBE_RADIUS",&rc)) {
        printf("ERROR: %s::read_config_file_key(TUBE_RADIUS) returned error\n",argv[0]);
        return EXIT_FAILURE;
    }
    TUBE_RADIUS = (float)atof(rc.c_str());
    printf("# DBG:%s:TUBE_RADIUS=%f\n",argv[0],TUBE_RADIUS);

    if ((TUBE_RADIUS == -1.0)) {    // (MIN_CHANNEL_MM == -1.0) || 
        printf("ERROR: %s::read_config_file_key(): config values were not loaded from CFG_FILE. Is it in this PWD?\n",argv[0]);
        return EXIT_FAILURE;
    }
    
    int argc_cnt = 1;
    
    if (argc == 1) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    bool binfo = false;
    std::string inp = argv[argc_cnt];
    if (inp == "-i") {
        argc_cnt++;
        binfo = true;
    }
    
    // hack!
    //printf("# DEBUGCFG:%s:argc=%d argc_cnt=%d\n",argv[0],argc,argc_cnt);
    if ( (argc-argc_cnt+1) <= 3 ) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    // open STL
    std::string inputFilename = argv[argc_cnt++];
    printf("# DEBUGCFG:%s:IN STL = %s\n",argv[0],inputFilename.c_str());
    if (!read_stl(&reader,inputFilename,true))
        return -1;
    std::string pack_cube = argv[argc_cnt++];
    printf("# DEBUGCFG:%s:pack-cube=%s\n",argv[0],pack_cube.c_str());
    float cell_face_mm = atof(argv[argc_cnt++]);
    printf("# DEBUGCFG:%s:face_mm=%f\n",argv[0],cell_face_mm);
    Gcell_face_mm = cell_face_mm;
    
    get_polydata_info(&info,reader->GetOutput(),true);
    // if just here for info, we're done with that
    if (binfo) return 0;
    
    double SHELL_SCALE_inner = -1.0 * (TUBE_RADIUS/1.0);
    printf("# MODEL: scaling input STL slightly by %f to accomodate tube radius\n",SHELL_SCALE_inner);
    
    // config enclosed points
    config_enclosedPoints_inner(SHELL_SCALE_inner);
    
    // INIT: closest points in reader() STL data
    cellLocator->SetDataSet(stl_inner_shell);
    cellLocator->BuildLocator();

	// Create a container with the geometry given above, and make it
	// non-periodic in each of the three coordinates. Allocate space for
	// eight particles within each computational block.
    container con(info.min.x,info.max.x,info.min.y,info.max.y,info.min.z,info.max.z,n_x,n_y,n_z,
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
#elif CFG_GENERATE_WALL_FROM_STL
    // create wall of STL, centered on it
    wall_user_callback wall(info.center[0],info.center[1],info.center[2],&voro_cb_cut_cell,&voro_cb_point_inside);
    con.add_wall(wall);
#else
#error "ERROR: define a CFG_GENERATE_xxx define"
#endif
    
    con.import(pack_cube.c_str());
    con.draw_cells_gnuplot("cad_small.gnu");
	con.print_custom("%B","cad_small-cylinder.csv");
//    con.draw_particles();
    
    // For debug: output STL
//    if (!write_stl(inputFilename,stl_inner_shell,"voro_reduced",true))
    //  return -1;
    
    return 0;
}






void config_enclosedPoints_inner(double scale) {
    
#if 0
//    vtkSmartPointer<vtkPolyData> temppolydata = vtkSmartPointer<vtkPolyData>::New();
//    temppolydata = reader->GetOutput();
//
//    // Create the locator
//    vtkSmartPointer<vtkOBBTree> tree = vtkSmartPointer<vtkOBBTree>::New();
//    tree->SetDataSet(temppolydata);
//    tree->BuildLocator();
//    
//    // call fcn, voro_fill also needs this
//    int ret = -1;
//    vtkSmartPointer<vtkPolyData> temppolydata2 = vtkSmartPointer<vtkPolyData>::New();
//    temppolydata2 = extrude_polydata_along_normal(temppolydata,scale,tree,MIN_CHANNEL_MM,true,info,&ret);
//    if (ret != 0) {
//        printf("ERROR: error returned from extrude_polydata_along_normal()\n");
//        return;// ret;
//    }
#else
    vtkSmartPointer<vtkPolyData> temppolydata2 = vtkSmartPointer<vtkPolyData>::New();
    temppolydata2 = reader->GetOutput();
#endif
    
    // for the inner surface
    vtkSmartPointer<vtkPolyDataNormals> pnormal = vtkSmartPointer<vtkPolyDataNormals>::New();
#if VTK_MAJOR_VERSION <= 5
    pnormal->SetInput(temppolydata2);
#else
    pnormal->SetInputData(temppolydata2);
#endif
    pnormal->SplittingOff();
    pnormal->FlipNormalsOff();
    pnormal->ComputeCellNormalsOn();
    pnormal->ComputePointNormalsOff();
    pnormal->Update();
    
    stl_inner_shell = pnormal->GetOutput();
    selectEnclosedPoints->Initialize(stl_inner_shell);
    selectEnclosedPoints->SetTolerance(0.0000001);
    //    selectEnclosedPoints->Update();
}

// TODO: return false if not implemented yet
bool GetCellNormals(vtkPolyData* polydata, vtkIdType cellId, double* cN)
{
    //    std::cout << "Looking for cell normals..." << std::endl;
    //
    //    // Count points
    //    vtkIdType numCells = polydata->GetNumberOfCells();
    //    std::cout << "There are " << numCells << " cells." << std::endl;
    //
    //    // Count triangles
    //    vtkIdType numPolys = polydata->GetNumberOfPolys();
    //    std::cout << "There are " << numPolys << " polys." << std::endl;
    
    ////////////////////////////////////////////////////////////////
    // Double normals in an array
    vtkDoubleArray* normalDataDouble = vtkDoubleArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));
    
    if(normalDataDouble)
    {
        int nc = normalDataDouble->GetNumberOfTuples();
        std::cout << "There are " << nc
            << " components in normalDataDouble" << std::endl;
        return false;//true;
    }
    
    ////////////////////////////////////////////////////////////////
    // Double normals in an array
    vtkFloatArray* normalDataFloat = vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetArray("Normals"));
    
    if(normalDataFloat)
    {
        int nc = normalDataFloat->GetNumberOfTuples();
        //        std::cout << "There are " << nc << " components in normalDataFloat" << std::endl;
        normalDataFloat->GetTuple(cellId, cN);
        //        cout << "Cell normal " << cellId << ": " << cN[0] << " " << cN[1] << " " << cN[2] << endl;
        return true;
    }
    
    ////////////////////////////////////////////////////////////////
    // Point normals
    vtkDoubleArray* normalsDouble = vtkDoubleArray::SafeDownCast(polydata->GetCellData()->GetNormals());
    
    if(normalsDouble)
    {
        std::cout << "There are " << normalsDouble->GetNumberOfComponents()
            << " components in normalsDouble" << std::endl;
        return false;//true;
    }
    
    ////////////////////////////////////////////////////////////////
    // Point normals
    vtkFloatArray* normalsFloat = vtkFloatArray::SafeDownCast(polydata->GetCellData()->GetNormals());
    
    if(normalsFloat)
    {
        std::cout << "There are " << normalsFloat->GetNumberOfComponents()
            << " components in normalsFloat" << std::endl;
        return false;//true;
    }
    
    /////////////////////////////////////////////////////////////////////
    // Generic type point normals
    vtkDataArray* normalsGeneric = polydata->GetCellData()->GetNormals(); //works
    if(normalsGeneric)
    {
        std::cout << "There are " << normalsGeneric->GetNumberOfTuples()
            << " normals in normalsGeneric" << std::endl;
        
        double testDouble[3];
        normalsGeneric->GetTuple(0, testDouble);
        
        std::cout << "Double: " << testDouble[0] << " "
            << testDouble[1] << " " << testDouble[2] << std::endl;
        
        // Can't do this:
        /*
         float testFloat[3];
         normalsGeneric->GetTuple(0, testFloat);
         
         std::cout << "Float: " << testFloat[0] << " "
         << testFloat[1] << " " << testFloat[2] << std::endl;
         */
        return false;//true;
    }
    
    
    // If the function has not yet quit, there were none of these types of normals
    std::cout << "Normals not found!" << std::endl;
    return false;
    
}






