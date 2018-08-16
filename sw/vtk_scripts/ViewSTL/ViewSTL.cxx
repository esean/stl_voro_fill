//
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
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCamera.h>

//#define VIEW_INTERESTING

int main ( int argc, char *argv[] )
{
    
  if ( argc < 2 )
    {
        shared_print_version();
        cout << "Required parameters: {[-w|-t]} [STL filename]" << endl;
        cout << "Optional arg {[-w]} shows model in wireframe view" << endl;
        cout << "Optional arg {[-t]} runs in non-trackpad style" << endl;
        return EXIT_FAILURE;
    }

    int argc_cnt = 1;
    bool bdoWF = false;
    std::string inp = argv[argc_cnt];
    if (inp == "-w") {
        argc_cnt++;
        bdoWF = true;
    }
    bool bdoNTP = false;
    inp = argv[argc_cnt];
    if (inp == "-t") {
        argc_cnt++;
        bdoNTP = true;
    }
    std::string inputFilename = argv[argc_cnt++];
    std::string window_title;
    window_title += inputFilename;
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    if (!read_stl(&reader,inputFilename,false))
      return -1;
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
      polyData = reader->GetOutput();
    
    std::string inputFilename2;
    bool bSecFn = false;
    vtkSmartPointer<vtkSTLReader> reader2 = vtkSmartPointer<vtkSTLReader>::New();
    vtkSmartPointer<vtkPolyData> polyData2 = vtkSmartPointer<vtkPolyData>::New();
    if (argc != argc_cnt) {
        inputFilename2 = argv[argc_cnt++];
        window_title = window_title + ", " + inputFilename2;
        bSecFn = true;
        if (!read_stl(&reader2,inputFilename2,false))
          return -1;
        polyData2 = reader2->GetOutput();
    }
    
  // Visualize
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(reader->GetOutputPort());
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->GetProperty()->SetColor(0.9,0.9,0.1);
  actor->SetMapper(mapper);
    if (bdoWF)
        actor->GetProperty()->SetRepresentationToWireframe();
    actor->GetProperty()->SetOpacity(1.0);
    
    vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
    vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkActor>::New();
    if (bSecFn) {
        mapper2->SetInputConnection(reader2->GetOutputPort());
        actor2->SetMapper(mapper2);
        actor2->GetProperty()->SetColor(0.9,0.1,0.5);
        actor->GetProperty()->SetOpacity(0.5);
        actor2->GetProperty()->SetOpacity(1.0);
        if (bdoWF)
            actor2->GetProperty()->SetRepresentationToWireframe();
    }
    
    
  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName(window_title.c_str()); // Set the title
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->AddActor(actor2);
  renderer->SetBackground(.3, .6, .3); // Background color green

    if (!bdoNTP) {
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    renderWindowInteractor->SetInteractorStyle( style );
    }
    
    // Generate an interesting view
    //
#ifdef VIEW_INTERESTING
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Azimuth(120);
    renderer->GetActiveCamera()->Elevation(30);
    renderer->GetActiveCamera()->Dolly(1.0);
    renderer->ResetCameraClippingRange();
#endif
    
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
