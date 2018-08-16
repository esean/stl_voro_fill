#include "../common/vtk_common.h"
// write stl
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
// OBB
#include <vtkLine.h>
#include <vtkPolyDataNormals.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkSphereSource.h>
#include <vtkLineSource.h>
#include <vtkProperty.h>
#include <vtkDecimatePro.h>
#include <cstdio>

vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();

void usage(char* str)
{
    shared_print_version();
    printf("\n");
    printf("USAGE: %s {[ARGS]} [STL image.stl] [set triangle count]\n",str);
    printf("\n");
    printf("Reduce triangle count of STL surface, writes out STL result.\n");
    printf("\n");
    printf("  {ARGS}\n");
    printf("      -i		just show info about dataset, don't reduce\n");
    printf("\n");
    printf("Required params:\n");
    printf("\n");
    printf("    [STL image]        --> user-provided 3d model\n");
    printf("    [set triangle count]         --> Set desired triangle count\n");
    printf("\n");
}


int main ( int argc, char *argv[] )
{
    printf("ERROR: don't use this vtk.app, instead use mm_api_remesh.py\n");
    exit(-1);
    
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
    int max_args = binfo ? 1 : 2;
    if ( (argc-argc_cnt+1) <= max_args ) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    std::string inputFilename = argv[argc_cnt++];
    printf("# DEBUGCFG:%s:IN STL = %s\n",argv[0],inputFilename.c_str());
    long long int reduce_tri = 0;
    if (!binfo) {
        reduce_tri = atoll(argv[argc_cnt++]);
        printf("# DEBUGCFG:%s:set triangle count=%lld\n",argv[0],reduce_tri);
    }
    
    // open STL
    if (!read_stl(&reader,inputFilename,true)) {
        fprintf(stderr,"ERROR:%s: Failed to read in STL file:%s\n",argv[0],inputFilename.c_str());
        return EXIT_FAILURE;
    }
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData = reader->GetOutput();
    
    model_info info;
    get_polydata_info(&info,polyData,true);
    double xmax = info.max.x-info.min.x;
    double ymax = info.max.y-info.min.y;
    double zmax = info.max.z-info.min.z;
    float reduce_percentage = 1.0 - (float)reduce_tri / (float)polyData->GetNumberOfCells();
    
    // display init
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);
    
    std::cout << "# before " << polyData->GetNumberOfPoints() << " points" << std::endl;
    std::cout << "# before " << polyData->GetNumberOfPolys() << " polygons" << std::endl;
    
    if (binfo) return EXIT_SUCCESS;
    
    
    
    
    vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
#if VTK_MAJOR_VERSION <= 5
    decimate->SetInputConnection(polyData->GetProducerPort());
#else
    decimate->SetInputData(polyData);
#endif
    decimate->SetTargetReduction(reduce_percentage); //10% reduction (if there was 100 triangles, now there will be 90)
    decimate->Update();
    
    vtkSmartPointer<vtkPolyData> decimated = vtkSmartPointer<vtkPolyData>::New();
    decimated->ShallowCopy(decimate->GetOutput());
    
    std::cout << "# after " << decimated->GetNumberOfPoints() << " points" << std::endl;
    std::cout << "# after " << decimated->GetNumberOfPolys() << " polygons" << std::endl;
    
    
//    vtkSmartPointer<vtkSmoothPolyDataFilter> smoothFilter = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
////    smoothFilter->SetInputConnection(polyData->GetOutput());
//    smoothFilter->SetInputData(polyData);//->GetOutput());
//    smoothFilter->SetNumberOfIterations(15);
//    smoothFilter->SetRelaxationFactor(0.5);
//    smoothFilter->FeatureEdgeSmoothingOn();
//    smoothFilter->BoundarySmoothingOn();
//    smoothFilter->Update();

    // now do clean filter to merge duplicate points, and/or remove unused
    //    points and/or remove degenerate cells
    vtkSmartPointer<vtkCleanPolyData> cleanPolyData = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanPolyData->SetInputConnection(decimate->GetOutputPort());
    cleanPolyData->Update();
    
    
    if (!write_stl(inputFilename,decimate->GetOutput(),"remesh",true))
        return -1;
    
//    if (!binfo) {
//        // Visualize
//        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//        mapper->SetInputConnection(cleanPolyData->GetOutputPort());
//        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
//        actor->GetProperty()->SetColor(0.5,1,0.5);
////        actor->GetProperty()->SetOpacity(0.5);
//        actor->SetMapper(mapper);
//        renderer->AddActor(actor);
//        
//        renderer->SetBackground(.3, .6, .3); // Background color green
//        renderWindow->Render();
//        renderWindowInteractor->Start();
//    }
    
  return EXIT_SUCCESS;
}
