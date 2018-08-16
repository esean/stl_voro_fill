//
#include "../common/vtk_common.h"
#include <vtkHexahedron.h>
#include <vtkUnstructuredGrid.h>
#include <vtkGeometryFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>

int main (int argc, char *argv[])
{
    if ( argc < 5 )
    {
        shared_print_version();
        cout << "\nUSAGE: " << argv[0] << " [STL] [ARGS]" << endl;
        cout << "\nRotates STL" << endl;
        cout << "\nARGS are {[degX][degY][degZ]" << endl; // [rotX][rotY][rotZ]}'" << endl;
        cout << "  where [degXYZ] are the angles in degrees to rotate, defaulting to 0deg," << endl;
//        cout << "  and {[rotXYZ]} is the (x,y,z) location to rotate around, defaulting to (0,0,0)" << endl;
        cout << "\nOUTPUT: writes to [SHAPE]-rot.stl" << endl;
        cout << endl;
        return EXIT_FAILURE;
    }

    
    printf("ERROR: this program is deprecated. Please use mm_api_transform.py instead\n");
    return EXIT_FAILURE;
    
    
    int argc_cnt = 1;
    std::string inputFilename = argv[argc_cnt++];
    // deg{XYZ} are the angle in degrees
    double degX = atof(argv[argc_cnt++]);
    double degY = atof(argv[argc_cnt++]);
    double degZ = atof(argv[argc_cnt++]);
//    double rotX = 0;
//    if (argc >= 9) rotX = atof(argv[argc_cnt++]);
//    double rotY = 0;
//    if (argc >= 9) rotY = atof(argv[argc_cnt++]);
//    double rotZ = 0;
//    if (argc >= 9) rotZ = atof(argv[argc_cnt++]);
    

    // load polygon input
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    if (!read_stl(&reader,inputFilename,false))
      return -1;
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
      polyData = reader->GetOutput();
    
    
    // before rotation, put item at center of x,y,z
    
//    // make polydata so we can find center, needed to rotating about a point
//    vtkSmartPointer<vtkPolyData> polyData2 = vtkSmartPointer<vtkPolyData>::New();
//    polyData2 = geometryFilter->GetOutput();
//    model_info mi;
//    get_polydata_info(&mi,polyData2,false);
//    printf("center=%f,%f,%f\n",mi.center[0],mi.center[1],mi.center[2]);

    
   // Set up the transform filter
  vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
//    double delXlate[3] = {0,0,0};
    translation->RotateX(degX);
    translation->RotateY(degY);
    translation->RotateZ(degZ);
 
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputData(polyData); // ->GetOutputPort());
  transformFilter->SetTransform(translation);
  transformFilter->Update();
    
    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputConnection(transformFilter->GetOutputPort());
    triangleFilter->Update();
    
    vtkSmartPointer<vtkPolyData> polyData_out = vtkSmartPointer<vtkPolyData>::New();
    polyData_out = triangleFilter->GetOutput();
    
  // write out and remesh before cleaning
  if (!write_stl(inputFilename,polyData_out,"rot",true,false))
      return -1;
    
  return EXIT_SUCCESS;
}
