#include "../common/vtk_common.h"
#include <vtkSmartPointer.h>
#include <vtkPLYReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkPointSource.h>
#include <vtkSTLWriter.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtksys/SystemTools.hxx>
#include <vtkTriangleFilter.h>
#include <vtkCleanPolyData.h>
#include <cstdio>

static vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);

void usage(char* str)
{
    shared_print_version();
    printf("USAGE: %s [input filename]\n",str);
    printf("Converts [input filename] to STL and writes out.\n");
}

int main (int argc, char *argv[])
{
    if (argc == 1) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    std::string infile(argv[1]);
    vtkSmartPointer<vtkPolyData> polyData = ReadPolyData(infile.c_str());
    if (!write_stl(infile,polyData,"STL",true))
        return -1;
    return EXIT_SUCCESS;
}

static vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension = vtksys::SystemTools::GetFilenameExtension(std::string(fileName));
  if (extension == ".ply")
  {
    vtkSmartPointer<vtkPLYReader> reader =
      vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
      vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
      triangleFilter->SetInputConnection(reader->GetOutputPort());
      triangleFilter->Update();
      polyData = triangleFilter->GetOutput();
  }
  else if (extension == ".vtp")
  {
    vtkSmartPointer<vtkXMLPolyDataReader> reader =
      vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
      vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
      triangleFilter->SetInputConnection(reader->GetOutputPort());
      triangleFilter->Update();
      polyData = triangleFilter->GetOutput();
  }
  else if (extension == ".obj")
  {
    vtkSmartPointer<vtkOBJReader> reader =
      vtkSmartPointer<vtkOBJReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
      vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
      triangleFilter->SetInputConnection(reader->GetOutputPort());
      triangleFilter->Update();
      polyData = triangleFilter->GetOutput();
  }
  else if (extension == ".stl")
  {
    vtkSmartPointer<vtkSTLReader> reader =
      vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName (fileName);
    reader->Update();
      vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
      triangleFilter->SetInputConnection(reader->GetOutputPort());
      triangleFilter->Update();
      polyData = triangleFilter->GetOutput();
  }
  else
  {
      fprintf(stderr,"ERROR: unsupported file type:%s\n",extension.c_str());
  }
  return polyData;
}
