//
#include "../common/vtk_common.h"
#include <vtkCleanPolyData.h>

int main ( int argc, char *argv[] )
{
    bool display = false;
  if ( argc != 2 )
    {
        shared_print_version();
    cout << "Required parameters: [STL] " << endl;
        cout << "Cleans STL" << endl;
        cout << "OUTPUT: writes to [STL]-clean.stl" << endl;
    return EXIT_FAILURE;
    }
    
  int argc_cnt = 1;
  std::string inputFilename = argv[argc_cnt++];
    printf("# DEBUGCFG:%s:IN STL = %s\n",argv[0],inputFilename.c_str());
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    
    //
    // WARNING: DO NOT CLEAN STL (we are already doing that!)
    //
    if (!read_stl(&reader,inputFilename,false))
        return -1;
    
    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
        cleanFilter->SetInputConnection(reader->GetOutputPort());
        cleanFilter->Update();
    
    vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
    out = cleanFilter->GetOutput();
    
    //
    // WARNING: DO NOT CLEAN STL (we are already doing that!)
    //
    if (!write_stl(inputFilename,out,"clean",false))
        return -1;

  return EXIT_SUCCESS;
}
