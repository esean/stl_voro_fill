//
#include "../common/vtk_common.h"
#include <vtkPolyLine.h>
#include <vtkCamera.h>

int parse_line(char *ln, aPt &p1, aPt &p2, aPt &p3) {
    return sscanf(ln,"%f,%f,%f,%f,%f,%f,%f,%f,%f", &p1.x, &p1.y, &p1.z,
                  &p2.x, &p2.y, &p2.z, &p3.x, &p3.y, &p3.z);
}

int min(int x, int y) {
    return (x > y) ? y : x;
}
int max(int x, int y) {
    return (x < y) ? y : x;
}

void usage(char* str) {
    shared_print_version();
    printf("\nUSAGE: %s [FORM INPUT STL] [IN 9pt CSV]\n",str);
    printf("\nPlots 9-pt (pt->pt->pt) data in [IN 9pt CSV] and outputs three (side,front,top) PNGs to [IN 9pt CSV]-[angle].png.\n");
    printf("Also overlays a transparent image of data in [FORM INPUT STL]. That file should be STL that generated [IN 9pt CSV]\n");
    printf("\nExample:\n");
    printf("\t%s input_form.stl cad_small-cylinder.csv\n",str);
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    int argc_cnt = 1;
    
    std::string in_stl = argv[argc_cnt++];
    printf("# DEBUGCFG: IN STL = %s\n",in_stl.c_str());
    std::string in_fn = argv[argc_cnt++];
    FILE* file = fopen(in_fn.c_str(),"r");
    if (!file) {
        fprintf(stderr, "ERROR: file(%s) could not open file!\n",in_fn.c_str());
        return 2;
    }
    printf("# DEBUGCFG: IN FILE = %s\n",in_fn.c_str());
    
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    if (!read_stl(&reader,in_stl,true))
        return -1;
    vtkSmartPointer<vtkPolyData> polydata_stl = vtkSmartPointer<vtkPolyData>::New();
    polydata_stl = reader->GetOutput();
    
    // Setup render window, renderer, and interactor
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindow->AddRenderer(renderer);
    renderWindowInteractor->SetRenderWindow(renderWindow);
    
    
    // Create a polydata to store everything in
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    // Create a cell array to store the lines in and add the lines to it
    vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
    
    char line[1024];
    bool first_line = true;
    int pts = 0;
    
    while (fgets(line, sizeof(line), file)) {
        for (uint16_t i=0; i<strlen(line); ++i)
            if ((line[i] == '\r') || (line[i] == '\n')) line[i] = 0;
        aPt p1,p2,p3;
        int cnt = parse_line(line,p1,p2,p3);
        if (cnt == 9)
        {
            if (first_line) {
                // add pts p1,p2,p3
                points->InsertNextPoint(p1.x,p1.y,p1.z);
                points->InsertNextPoint(p2.x,p2.y,p2.z);
                points->InsertNextPoint(p3.x,p3.y,p3.z);
                pts += 3;
            } else {
                // add p3, only it changes for each new line after first
                points->InsertNextPoint(p3.x,p3.y,p3.z);
                pts += 1;
            }
            first_line = false;
        }
        else if (cnt == 1)
        {
            vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
            polyLine->GetPointIds()->SetNumberOfIds(pts);
            for(unsigned int i = 0; i < pts; i++)
                polyLine->GetPointIds()->SetId(i,i);
            cells->InsertNextCell(polyLine);
            
            // done with face
            first_line = true;
        }
        else if (cnt == -1)
        {
            // done with cell
            first_line = true;
        }
    }
    
    // Add the points to the dataset
    polyData->SetPoints(points);
    // Add the lines to the dataset
    polyData->SetLines(cells);
    

    //
    //-----------------------------------
    // Viz result
    //-----------------------------------
    //
    
    // Visualize STL
    vtkSmartPointer<vtkPolyDataMapper> mapper_stl = vtkSmartPointer<vtkPolyDataMapper>::New();
#if VTK_MAJOR_VERSION <= 5
    mapper_stl->SetInput(polydata_stl);
#else
    mapper_stl->SetInputData(polydata_stl);
#endif
    mapper_stl->ScalarVisibilityOn();
    vtkSmartPointer<vtkActor> actor_stl = vtkSmartPointer<vtkActor>::New();
    actor_stl->SetMapper(mapper_stl);
    actor_stl->GetProperty()->SetOpacity(0.2);
    renderer->AddActor(actor_stl);
    
    
    // Visualize 9pt CSV
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
#if VTK_MAJOR_VERSION <= 5
    mapper->SetInput(polyData);
#else
    mapper->SetInputData(polyData);
#endif
    mapper->ScalarVisibilityOn();
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(255,0,0);
    renderer->AddActor(actor);
    renderer->SetBackground(.3, .6, .3); // Background color green
    renderWindow->Render();
    
    renderer->ResetCamera();
    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    camera = renderer->GetActiveCamera();
//    std::cout << "\tPosition: "
//    << camera->GetPosition()[0] << ", "
//    << camera->GetPosition()[1] << ", "
//    << camera->GetPosition()[2] << std::endl;
//    std::cout << "\tFocal point: "
//    << camera->GetFocalPoint()[0] << ", "
//    << camera->GetFocalPoint()[1] << ", "
//    << camera->GetFocalPoint()[2] << std::endl;
    
    // Screenshot
    renderWindow->Render();
    take_screenshot_renderer(&renderWindow,in_fn + "-side.png",5);

    camera->Azimuth(45);
    camera->Elevation(45);
    renderWindow->Render();
    take_screenshot_renderer(&renderWindow,in_fn + "-angle.png",5);
    // set back
    camera->Elevation(-45);
    camera->Azimuth(-45);
    
    camera->Azimuth(90);
    renderWindow->Render();
    take_screenshot_renderer(&renderWindow,in_fn + "-front.png",5);

    camera->Elevation(90);
    renderWindow->Render();
    take_screenshot_renderer(&renderWindow,in_fn + "-top.png",5);
    
    // open render window
//    renderWindowInteractor->Start();
    
    
    
//    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
//    triangleFilter->SetInputConnection(polyData->GetOutput());
//    triangleFilter->Update();
//    
//    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
//    cleanFilter->SetInputConnection(triangleFilter->GetOutputPort());
//    cleanFilter->Update();
//    
//    vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
//    out = cleanFilter->GetOutput();
    
//    if (!write_stl(in_fn,polyData,"9pt",false))
    //  return -1;
    
    return EXIT_SUCCESS;
}
