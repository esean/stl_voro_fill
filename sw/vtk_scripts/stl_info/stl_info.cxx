//
#include "../common/vtk_common.h"

int main ( int argc, char *argv[] )
{
    bool display = false;
  if ( argc != 2 )
    {
        shared_print_version();
    cout << "Required parameters: [STL]" << endl;
        cout << "Displays information about [STL]." << endl;
    return EXIT_FAILURE;
    }

  int argc_cnt = 1;
  std::string inputFilename = argv[argc_cnt++];
    printf("# DEBUGCFG:%s:IN STL = %s\n",argv[0],inputFilename.c_str());
    
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    if (!read_stl(&reader,inputFilename,false))
        return -1;
    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata = reader->GetOutput();

    

    if (is_stl_watertight(polydata)) {
        printf("# input STL %s is watertight\n",inputFilename.c_str());
    } else {
        printf("WARNING: input STL %s it NOT watertight\n",inputFilename.c_str());
        // TODO: fix it with with fix_stl.sh, that fix watertight?
    }
    
    model_info info;
    get_polydata_info(&info,polydata,true);
    
    float vol = 0.0;
    if (!get_stl_volume(inputFilename,&vol)) {
        printf("# MODEL: volume_mL = %f\n",vol);
    }
    
  return EXIT_SUCCESS;
}

