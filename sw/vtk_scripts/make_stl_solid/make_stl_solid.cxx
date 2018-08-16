//
#include "../common/vtk_common.h"
#include <vtkCleanPolyData.h>
#include <vtkFillHolesFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyDataConnectivityFilter.h>
//#include <vtkDataSetSurfaceFilter.h>
//#include <vtkGeometryFilter.h>
//#include <vtkDelaunay3D.h>
#include <vtkSurfaceReconstructionFilter.h>
#include <vtkContourFilter.h>
#include <vtkReverseSense.h>


// TODO: not working don't think


int main ( int argc, char *argv[] )
{
    shared_print_version();
    printf("\n#########################################################\n");
    printf("ERROR:%s is not working yet - port call to mm_api_make_solid.py...\n",argv[0]);
    printf("#########################################################\n\n");
    return EXIT_FAILURE;
    
    
    bool display = false;
  if ( argc != 2 )
    {
    cout << "Required parameters: [STL]" << endl;
        cout << "Make [STL] a solid." << endl;
        cout << "OUTPUT: writes to [STL]-solid.stl" << endl;
    return EXIT_FAILURE;
    }

  int argc_cnt = 1;
  std::string inputFilename = argv[argc_cnt++];
    printf("# DEBUGCFG:%s:IN STL = %s\n",argv[0],inputFilename.c_str());
    
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    if (!read_stl(&reader,inputFilename,true))
        return -1;
    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata = reader->GetOutput();

    

    if (is_stl_watertight(polydata)) {
        printf("# STL %s is watertight\n",inputFilename.c_str());
    } else {
        printf("WARNING: STL %s it NOT watertight\n",inputFilename.c_str());
    }
    
    
    
//    
//    vtkSmartPointer<vtkFillHolesFilter> fillHolesFilter = vtkSmartPointer<vtkFillHolesFilter>::New();
//#if VTK_MAJOR_VERSION <= 5
//    fillHolesFilter->SetInputConnection(polydata->GetProducerPort());
//#else
//    fillHolesFilter->SetInputData(polydata);
//#endif
//    fillHolesFilter->SetHoleSize(1000.0);
//
//    // Make the triangle windong order consistent
//    vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
//    normals->SetInputConnection(fillHolesFilter->GetOutputPort());
//    normals->ConsistencyOn();
//    normals->SplittingOff();
//    normals->Update();
//
//    // Restore the original normals
//    normals->GetOutput()->GetPointData()->SetNormals(polydata->GetPointData()->GetNormals());
//    
//    
//    
//    
//    
//    
//    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
////    triangleFilter->SetInputConnection(reader->GetOutputPort());
//    triangleFilter->SetInputConnection(normals->GetOutputPort());
//    triangleFilter->Update();
//    
//    vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
//    connectivityFilter->SetInputConnection(triangleFilter->GetOutputPort());
//    connectivityFilter->SetExtractionModeToAllRegions();
//    connectivityFilter->Update();
//    printf("##### FOUND regions %d\n",connectivityFilter->GetNumberOfExtractedRegions());
//    
//    // for each region, append each to polygon
//    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
//    
//    for (int i=0; i<connectivityFilter->GetNumberOfExtractedRegions(); ++i) {
//        
//        //        printf("#### ADD region %d\n",i);
//        vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter2 = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
//        connectivityFilter2->SetInputConnection(triangleFilter->GetOutputPort());
//        connectivityFilter2->SetExtractionModeToSpecifiedRegions();
//        connectivityFilter2->AddSpecifiedRegion(i); //select the region to extract here
//        connectivityFilter2->Update();
//#if VTK_MAJOR_VERSION <= 5
//        appendFilter->AddInputConnection(connectivityFilter2->GetOutputPort());
//#else
//        appendFilter->AddInputData(connectivityFilter2->GetOutput());
//#endif
//    }
//    appendFilter->Update();
//    
//    vtkSmartPointer<vtkUnstructuredGrid> unstructuredGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
//    unstructuredGrid->DeepCopy(appendFilter->GetOutput());
//
////    vtkSmartPointer<vtkDataSetSurfaceFilter> surfaceFilter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
////#if VTK_MAJOR_VERSION <= 5
////    surfaceFilter->SetInput(unstructuredGrid);
////#else
////    surfaceFilter->SetInputData(unstructuredGrid);
////#endif
////    surfaceFilter->SetPieceInvariant(1);
////    surfaceFilter->SetUseStrips(1);
////    surfaceFilter->Update();
//    
//    vtkSmartPointer<vtkGeometryFilter> geometryFilter = vtkSmartPointer<vtkGeometryFilter>::New();
//#if VTK_MAJOR_VERSION <= 5
//    geometryFilter->SetInput(unstructuredGrid);
//#else
//    geometryFilter->SetInputData(unstructuredGrid);
//#endif
//    printf("GetPointClipping = %d\n",geometryFilter->GetPointClipping());
//    printf("GetCellClipping = %d\n",geometryFilter->GetCellClipping());
//    printf("GetExtentClipping = %d\n",geometryFilter->GetExtentClipping());
//    printf("GetPointMinimum = %lld\n",geometryFilter->GetPointMinimum());
//    printf("GetPointMaximum = %lld\n",geometryFilter->GetPointMaximum());
//    printf("GetCellMinimum = %lld\n",geometryFilter->GetCellMinimum());
//    printf("GetCellMaximum = %lld\n",geometryFilter->GetCellMaximum());
//    printf("GetMerging = %d\n",geometryFilter->GetMerging());
//    geometryFilter->SetMerging(1);
//    geometryFilter->SetCellClipping(1);
//    geometryFilter->SetPointClipping(1);
//    geometryFilter->Update();
    
    
    
    
//    // Clean the polydata. This will remove duplicate points that may be
//    // present in the input data.
//    vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
//    cleaner->SetInputConnection (reader->GetOutputPort());
//    
//    // Generate a tetrahedral mesh from the input points. By
//    // default, the generated volume is the convex hull of the points.
//    vtkSmartPointer<vtkDelaunay3D> delaunay3D = vtkSmartPointer<vtkDelaunay3D>::New();
//    delaunay3D->SetInputConnection (cleaner->GetOutputPort());
//    
//    // Generate a mesh from the input points. If Alpha is non-zero, then
//    // tetrahedra, triangles, edges and vertices that lie within the
//    // alpha radius are output.
//    vtkSmartPointer<vtkDelaunay3D> delaunay3DAlpha = vtkSmartPointer<vtkDelaunay3D>::New();
//    delaunay3DAlpha->SetInputConnection (cleaner->GetOutputPort());
//    delaunay3DAlpha->SetAlpha(0.1);
//    
//    vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
//    polydata->DeepCopy(delaunay3DAlpha->GetOutput());
    
    
    
    
    // Construct the surface and create isosurface.
    vtkSmartPointer<vtkSurfaceReconstructionFilter> surf = vtkSmartPointer<vtkSurfaceReconstructionFilter>::New();
    
#if VTK_MAJOR_VERSION <= 5
    surf->SetInput(polydata);
#else
    surf->SetInputData(polydata);
#endif
    
    vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
    contourFilter->SetInputConnection(surf->GetOutputPort());
    contourFilter->SetValue(0, 0.0);
    
    // Sometimes the contouring algorithm can create a volume whose gradient
    // vector and ordering of polygon (using the right hand rule) are
    // inconsistent. vtkReverseSense cures this problem.
    vtkSmartPointer<vtkReverseSense> reverse = vtkSmartPointer<vtkReverseSense>::New();
    reverse->SetInputConnection(contourFilter->GetOutputPort());
    reverse->ReverseCellsOn();
    reverse->ReverseNormalsOn();
    reverse->Update();
    
    
    
//    vtkSmartPointer<vtkTriangleFilter> triangleFilter2 = vtkSmartPointer<vtkTriangleFilter>::New();
////    triangleFilter2->SetInputConnection(normals->GetOutputPort());
////    triangleFilter2->SetInputConnection(surfaceFilter->GetOutputPort());
//    triangleFilter2->SetInputConnection(delaunay3D->GetOutputPort());
//    triangleFilter2->Update();
//    
//    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
//        cleanFilter->SetInputConnection(delaunay3DAlpha->GetOutputPort());
//        cleanFilter->Update();
    
    vtkSmartPointer<vtkPolyData> out = vtkSmartPointer<vtkPolyData>::New();
//    out = delaunay3DAlpha->GetOutput(); //cleanFilter->GetOutput();
    out = reverse->GetOutput();
    
    if (!write_stl(inputFilename,polydata,"solid",true))
        return -1;

  return EXIT_SUCCESS;
}

