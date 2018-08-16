#include "../common/vtk_common.h"
#include <vtkPolyData.h>
#include <vtkSTLReader.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>

int main ( int argc, char *argv[] )
{
 
//    printf("argc=%d\n",argc);
  if ( argc != 5 )
    {
        shared_print_version();
    cout << "Required parameters: [STL] [x][y][z]" << endl;
        cout << "OUTPUT: writes to [STL]-xlate.stl" << endl;
    	return EXIT_FAILURE;
    }

  std::string inputFilename = argv[1];
  float tx = atof(argv[2]);
  float ty = atof(argv[3]);
  float tz = atof(argv[4]);
    
  vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    if (!read_stl(&reader,inputFilename,true))
        return -1;
    
    // Set up the transform filter
    
    vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
    translation->Translate(tx,ty,tz);
    
    vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformFilter->SetInputConnection(reader->GetOutputPort());
    transformFilter->SetTransform(translation);
    transformFilter->Update();

    // write to output STL 
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData = transformFilter->GetOutput();
    if (!write_stl(inputFilename, polyData, "xlate", true))
        return -1;
    
  return EXIT_SUCCESS;
}
