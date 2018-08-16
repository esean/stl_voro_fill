// todo:
//  - compute min/max distance across all faces, then we can use this to adjust knob to min needed

#define NmStl 1 // 1=just one STL putput, 4= output this number STL files
#define NmStl_Arr   4

#include "../common/vtk_common.h"
#include <vtkSmartPointer.h>
#include <vtkVersion.h>

#include <vtkParametricFunctionSource.h>
#include <vtkTupleInterpolator.h>
#include <vtkTubeFilter.h>
#include <vtkParametricSpline.h>

#include <vtkDoubleArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkTriangleFilter.h>
#include <vtkSTLWriter.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkCellLocator.h>

#include <sys/time.h>

#define CFG_DO_RENDER   0   // always gens an STL, but 1=display on screen, 0=not

#define FCN_RESOLUTION  2	// (3=low, 10=high perf ) defines how detailed are curves
#define PIPE_SIDES      2 // (10=low, 20=high perf) how many sides define outline of pipe
#define FCN_RESOLUTION_HP  3   // high-performance
#define PIPE_SIDES_HP      6   // high-performance

#define MIN_LCL_RAD     0.1


/////////////
// todo: make class
#define mUNDEF   -1000   // probably can't get that in model x,y,z
typedef struct {
    double sum;
    int cnt;
    double min,max;
} anAvg;
typedef struct {
    anAvg x,y,z;
} aPtAvg;
void aPtAvg_clear(aPtAvg *pt) {
    pt->x.sum = pt->x.cnt = 0.0;
    pt->x.min = pt->x.max = mUNDEF;
    pt->y.sum = pt->y.cnt = 0.0;
    pt->y.min = pt->y.max = mUNDEF;
    pt->z.sum = pt->z.cnt = 0.0;
    pt->z.min = pt->z.max = mUNDEF;
}
void aPtAvg_add_x(aPtAvg *pt, double sum) {
    pt->x.sum += sum;
    pt->x.cnt++;
    if ((sum > pt->x.max) || (pt->x.max == mUNDEF)) pt->x.max = sum;
    if ((sum < pt->x.min) || (pt->x.min == mUNDEF)) pt->x.min = sum;
}
void aPtAvg_add_y(aPtAvg *pt, double sum) {
    pt->y.sum += sum;
    pt->y.cnt++;
    if ((sum > pt->y.max) || (pt->y.max == mUNDEF)) pt->y.max = sum;
    if ((sum < pt->y.min) || (pt->y.min == mUNDEF)) pt->y.min = sum;
}
void aPtAvg_add_z(aPtAvg *pt, double sum) {
    pt->z.sum += sum;
    pt->z.cnt++;
    if ((sum > pt->z.max) || (pt->z.max == mUNDEF)) pt->z.max = sum;
    if ((sum < pt->z.min) || (pt->z.min == mUNDEF)) pt->z.min = sum;
}
void aPtAvg_add_xyz(aPtAvg *pt, double x, double y, double z) {
//    printf("### DBG: %f,%f,%f\n",x,y,z);
    aPtAvg_add_x(pt,x);
    aPtAvg_add_y(pt,y);
    aPtAvg_add_z(pt,z);
//    printf("### DBG:   delX = %f %f\n",pt->x.min,pt->x.max);
//    printf("### DBG:   delY = %f %f\n",pt->y.min,pt->y.max);
//    printf("### DBG:   delZ = %f %f\n",pt->z.min,pt->z.max);
}
void aPtAvg_get_avgs(aPtAvg *pt, double *x, double *y, double *z, double *delx, double *dely, double *delz) {
    *x = pt->x.sum / pt->x.cnt;
    *y = pt->y.sum / pt->y.cnt;
    *z = pt->z.sum / pt->z.cnt;
    *delx = pt->x.max - pt->x.min;
    *dely = pt->y.max - pt->y.min;
    *delz = pt->z.max - pt->z.min;
}
/////////////


//typedef struct {
//    float x,y,z;
//} aPt;

void find_midpoint_along_line_from(aPt p1, aPt p2, aPt &ret) {
    ret.x = (p1.x + p2.x)/2.0;
    ret.y = (p1.y + p2.y)/2.0;
    ret.z = (p1.z + p2.z)/2.0;
}

void find_spline_points(aPt p1, aPt p2, aPt p3,
                        aPt &ret1, aPt &ret2, aPt &ret3) {
    find_midpoint_along_line_from(p1,p2,ret1);
    find_midpoint_along_line_from(p2,p3,ret3);
    aPt xx;
    find_midpoint_along_line_from(ret1,ret3,xx);
    find_midpoint_along_line_from(xx,p2,ret2);
}

void calc_stat_spline(aPtAvg *aptavg, aPt p1, aPt p2, aPt p3, aPt *min, aPt *max, aPt *ret_middle) {
//    printf("calc_stat_spline(%f,%f,%f,%f,%f,%f,%f,%f,%f);\n",p1.x,p1.y,p1.z,p2.x,p2.y,p2.z,p3.x,p3.y,p3.z);
    max->x = std::max(max->x,p1.x);
    max->x = std::max(max->x,p2.x);
    max->x = std::max(max->x,p3.x);
    min->x = std::min(min->x,p1.x);
    min->x = std::min(min->x,p2.x);
    min->x = std::min(min->x,p3.x);
    
    max->y = std::max(max->y,p1.y);
    max->y = std::max(max->y,p2.y);
    max->y = std::max(max->y,p3.y);
    min->y = std::min(min->y,p1.y);
    min->y = std::min(min->y,p2.y);
    min->y = std::min(min->y,p3.y);
    
    max->z = std::max(max->z,p1.z);
    max->z = std::max(max->z,p2.z);
    max->z = std::max(max->z,p3.z);
    min->z = std::min(min->z,p1.z);
    min->z = std::min(min->z,p2.z);
    min->z = std::min(min->z,p3.z);
    
    // add_spline()...
    
//    p1.x *= scaling; p1.y *= scaling; p1.z *= scaling;
//    p2.x *= scaling; p2.y *= scaling; p2.z *= scaling;
//    p3.x *= scaling; p3.y *= scaling; p3.z *= scaling;
    aPt r1,r2,r3;
    find_spline_points(p1,p2,p3,r1,r2,r3);
//    points->InsertNextPoint(r1.x,r1.y,r1.z);
//    points->InsertNextPoint(r2.x,r2.y,r2.z);
//    points->InsertNextPoint(r3.x,r3.y,r3.z);
    
    ret_middle->x = r2.x;
    ret_middle->y = r2.y;
    ret_middle->z = r2.z;
    
    aPtAvg_add_xyz(aptavg,r2.x, r2.y, r2.z);
}

void add_spline(vtkPoints *points, double scaling, aPtAvg *aptavg, aPt p1, aPt p2, aPt p3, aPt *ret_middle) {
//    printf("add_spline(points,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f);\n",scaling,p1.x,p1.y,p1.z,p2.x,p2.y,p2.z,p3.x,p3.y,p3.z);
    p1.x *= scaling; p1.y *= scaling; p1.z *= scaling;
    p2.x *= scaling; p2.y *= scaling; p2.z *= scaling;
    p3.x *= scaling; p3.y *= scaling; p3.z *= scaling;
    aPt r1,r2,r3;
    find_spline_points(p1,p2,p3,r1,r2,r3);
    points->InsertNextPoint(r1.x,r1.y,r1.z);
    points->InsertNextPoint(r2.x,r2.y,r2.z);
    points->InsertNextPoint(r3.x,r3.y,r3.z);
    
    ret_middle->x = r2.x;
    ret_middle->y = r2.y;
    ret_middle->z = r2.z;
    
    aPtAvg_add_xyz(aptavg,r2.x, r2.y, r2.z);
}

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
        printf("\nUSAGE: %s {[OPTIONS]} [REQUIRED ARGS]\n",str);
        printf("\n{[OPTIONS]}:\n");
        printf("\t[-i [IN CSV]]\t  print stats about input [IN_CSV]\n");
        printf("\t[-p]\t  high performance render (anything for 3d printing)\n");
        printf("\n[REQUIRED ARGS]:\n");
        printf("\t[IN CSV] [inner pipe diameter] [outside pipe diameter scaling factor] [output STL name] [overall x,y,z scaling factor]\n");
        printf("Scaling is applied to point in [IN CSV] and to [pipe diameter].\n");
        printf("Example:\n");
        printf("\t%s -p  cad_small-cylinder.csv 1 0 $PWD/out.stl 1\n",str);
        printf("\n");
}

int main(int argc, char *argv[])
{
    struct timeval  tv1, tv2;
    gettimeofday(&tv1, NULL);

    if (argc == 1) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    int argc_cnt = 1;
    bool just_show_file_into_quit = false;
    if (!strcmp(argv[argc_cnt],"-i")) {
        argc_cnt++;
        just_show_file_into_quit = true;
    } else if ( argc < 6 ) {
	usage(argv[0]);
        return 1;
    }
    
    int mFCN_RESOLUTION = FCN_RESOLUTION;
    int mPIPE_SIDES = PIPE_SIDES;
    
    if (!strcmp(argv[argc_cnt],"-p")) {
        argc_cnt++;
        mFCN_RESOLUTION = FCN_RESOLUTION_HP;
        mPIPE_SIDES = PIPE_SIDES_HP;
    }
    
    char* in_fn = argv[argc_cnt++];
    FILE* file = fopen(in_fn,"r");
    if (!file) {
        fprintf(stderr, "ERROR: file(%s) could not open file!\n",in_fn);
        return 2;
    }
    printf("# DEBUGCFG: IN FILE = %s\n",in_fn);
    
    double rad;
    double pipe_rad_scaling;
    char* out_stl;
    double scaling = 1.0;
    if (!just_show_file_into_quit) {
        rad = atof(argv[argc_cnt++]);
        pipe_rad_scaling = atof(argv[argc_cnt++]);
        out_stl = argv[argc_cnt++];
        scaling = atof(argv[argc_cnt++]);
        rad *= scaling;
        printf("# DEBUGCFG: PIPE DIAMETER = %f\n",rad);
        printf("# DEBUGCFG: PIPE DIAMETER SCALING = %f\n",pipe_rad_scaling);
        printf("# DEBUGCFG: SCALING = %f x\n",scaling);
        printf("# DEBUGCFG: OUT STL = %s\n",out_stl);
        printf("# DEBUGCFG: NUMBER STL FILES OUTPUT = %d, array = %d\n",NmStl,NmStl_Arr);
    }
    
    // Setup render window, renderer, and interactor
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindow->AddRenderer(renderer);
    renderWindowInteractor->SetRenderWindow(renderWindow);
    vtkSmartPointer<vtkAppendPolyData> appendFilter[NmStl_Arr];
    for (int i=0; i < NmStl_Arr; ++i)
        appendFilter[i] = vtkSmartPointer<vtkAppendPolyData>::New();
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    
    
    //
    //-----------------------------------
    // calculate stats, file total spline/face/cell counts, min/max for each x/y/z so we know center axis vector
    //-----------------------------------
    //
    
    char line[1024];
    bool added_splines = false;
    int total_splines, total_cells, total_faces;
    total_splines = total_cells = total_faces = 0;
    aPt model_min = {0,0,0};
    aPt model_max = {0,0,0};
    aPtAvg avg_face_xyz;
    aPtAvg_clear(&avg_face_xyz);
    double max_dist;
    printf("## DELTAS(x,y,z,sz_vec): # x,y,z,sz_vec,face_min,face_max\n");
    aPt ret_middle;
    aPt start;
    double face_min = 1e+6;
    double face_max = 0;
    while (fgets(line, sizeof(line), file)) {
        for (uint16_t i=0; i<strlen(line); ++i)
            if ((line[i] == '\r') || (line[i] == '\n')) line[i] = 0;
        aPt p1,p2,p3;
        int cnt = parse_line(line,p1,p2,p3);
        if (cnt == 9)
        {
            // todo: should add scaling to this call to be consistent, then don't need mess below
            calc_stat_spline(&avg_face_xyz,p1,p2,p3,&model_min,&model_max,&ret_middle);
            
            if (!added_splines)
                start = ret_middle;
            else
            {
                double dx = ret_middle.x - start.x;
                double dy = ret_middle.y - start.y;
                double dz = ret_middle.z - start.z;
                double start_dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                face_min = (start_dist < face_min) ? start_dist : face_min;
                face_max = (start_dist > face_max) ? start_dist : face_max;
            }
            added_splines = true;
             total_splines++;
            
        }
        else if ( (cnt == 1) && added_splines)
        {
            added_splines = false;
            total_faces++;
            
            double x,y,z;
            double delx,dely,delz;
            aPtAvg_get_avgs(&avg_face_xyz, &x,&y,&z, &delx,&dely,&delz);
            // todo: mess referedto
            double dist = sqrt(x*x*scaling*scaling + y*y*scaling*scaling);
            aPtAvg_clear(&avg_face_xyz);
            if (dist > max_dist) max_dist = dist;
//            printf("## CALC: %f,%f,%f => %f (%f)\n",x,y,z,dist,max_dist);
            double sz_vec = sqrt(delx*delx + dely*dely + delz*delz);
            printf("## FACE %d DELTAS(x,y,z,sz_vec): %f,%f,%f,%f,%f,%f\n",total_faces,
                   delx,dely,delz,
                   sz_vec,
                   face_min,face_max);
            face_min = 1e+6;
            face_max = 0;
        }
        else if (cnt == -1)
        {
            added_splines = false;
            total_cells++;
        }
    }
    printf("# TOTAL splines = %d, faces = %d, cells = %d\n",total_splines,total_faces,total_cells);
    printf("# x(%f,%f) y(%f,%f) z(%f,%f) max_dist = %f\n",model_min.x,model_max.x,
           model_min.y,model_max.y,
           model_min.z,model_max.z,
           max_dist/scaling);
    if (just_show_file_into_quit) return(0);

    
    //
    //-----------------------------------
    // now draw the file
    //-----------------------------------
    //
    
    rewind(file);
    int now_splines, now_cells, now_faces;
    now_splines = now_cells = now_faces = 0;
    int print_stat_cnt = 0;
    added_splines = false;
    double min_tube_radius = 0.0;
    
    while (fgets(line, sizeof(line), file)) {
        for (uint16_t i=0; i<strlen(line); ++i)
            if ((line[i] == '\r') || (line[i] == '\n')) line[i] = 0;
        
        aPt p1,p2,p3;
        int cnt = parse_line(line,p1,p2,p3);
        if (cnt == 9)
        {
            added_splines = true;
            now_splines++;
            add_spline(points,scaling,&avg_face_xyz,p1,p2,p3,&ret_middle);
        }
        else if ( (cnt == 1) && added_splines)
        {
            added_splines = false;
//            printf("# draw all pipes\n");
            now_faces++;
            
            // Fit a spline to the points
            vtkSmartPointer<vtkParametricSpline> spline = vtkSmartPointer<vtkParametricSpline>::New();
            spline->SetPoints(points);
            vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
            functionSource->SetParametricFunction(spline);
            functionSource->SetUResolution(mFCN_RESOLUTION * points->GetNumberOfPoints());
            functionSource->Update();
            points->Reset();
            
            double lcl_rad = rad;
            double x,y,z;
            double delx,dely,delz;
            aPtAvg_get_avgs(&avg_face_xyz, &x,&y,&z, &delx,&dely,&delz);
            double dist = sqrt(x*x + y*y);
            // 012417: assumes model is a cylinder with half-sphere on top, when z is in
            //  that tip, reduce pipe radius for distance
            if (z > (model_max.z - x/2))
                dist = sqrt(x*x + y*y + z*z);
            lcl_rad = rad - (rad * pipe_rad_scaling) * (dist/max_dist);
//            printf("## %f,%f,%f => %f dist / %f max_dist = %f lcl_rad\n",x,y,z,dist,max_dist,lcl_rad);
            aPtAvg_clear(&avg_face_xyz);
            double sz_vec = sqrt(delx*delx + dely*dely + delz*delz);
            
            // Generate the radius scalars
            vtkSmartPointer<vtkDoubleArray> tubeRadius = vtkSmartPointer<vtkDoubleArray>::New();
            unsigned int n = functionSource->GetOutput()->GetNumberOfPoints();
            tubeRadius->SetNumberOfTuples(n);
            tubeRadius->SetName("TubeRadius");
            for (unsigned int i = 0; i < n; ++i)
                tubeRadius->SetTuple1(i, lcl_rad);
//            printf("# LCL_RAD %f\n",lcl_rad);
            min_tube_radius = (min_tube_radius == 0.0) ? lcl_rad : std::min(min_tube_radius,lcl_rad);
            if (lcl_rad < MIN_LCL_RAD) {
                fprintf(stderr,"ERROR: tube radius %f less than min value %f\n",lcl_rad,MIN_LCL_RAD);
                exit(1);
            }
            
            // Add the scalars to the polydata
            vtkSmartPointer<vtkPolyData> tubePolyData = vtkSmartPointer<vtkPolyData>::New();
            tubePolyData = functionSource->GetOutput();
            tubePolyData->GetPointData()->AddArray(tubeRadius);
            tubePolyData->GetPointData()->SetActiveScalars("TubeRadius");
            
            // todo: only need to compute the colors[] for which index of NmStl we are filling
            int nV = n;
            vtkSmartPointer<vtkUnsignedCharArray> colors[NmStl_Arr];
            for (int i=0; i < NmStl_Arr; ++i) {
                colors[i] = vtkSmartPointer<vtkUnsignedCharArray>::New();
                colors[i]->SetName("Colors");
                colors[i]->SetNumberOfComponents(3);
                colors[i]->SetNumberOfTuples(nV);
                int v,x,b;
            #if NmStl == 1
                v = 128;
                x = 191;
                b = 64;
            #else
                #define CLR_MIN 50
                double g = CLR_MIN + (((255 * 3)-CLR_MIN) * (double)i / (double)NmStl_Arr);
                v = min((int)g,255);
                x = min(max((int)g-255,0),255);
                b = min(max((int)g-255*2,0),255);
            #endif
                v = min(v,255);
                x = min(x,255);
                b = min(b,255);
//                printf("# DBG CLR %d: %d %d %d\n",i,v,x,b);
                for (int j = 0; j < nV ;j++)
                    colors[i]->InsertTuple3(j,v,x,b);
            }
  
            // Create the tubes
            vtkSmartPointer<vtkTubeFilter> tuber = vtkSmartPointer<vtkTubeFilter>::New();
            #if VTK_MAJOR_VERSION <= 5
            tuber->SetInput(tubePolyData);
            #else
            tuber->SetInputData(tubePolyData);
            #endif
            tuber->SetNumberOfSides(mPIPE_SIDES);
            tuber->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
            
            //--------------
            // Setup actors and mappers
            vtkSmartPointer<vtkPolyDataMapper> tubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            tubeMapper->SetInputConnection(tuber->GetOutputPort());
            tubeMapper->SetScalarRange(tubePolyData->GetScalarRange());
            vtkSmartPointer<vtkActor> tubeActor = vtkSmartPointer<vtkActor>::New();
            tubeActor->SetMapper(tubeMapper);
            vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
            triangleFilter->SetInputConnection(tuber->GetOutputPort());
            triangleFilter->Update();
            tubeMapper->ScalarVisibilityOn();
            tubeMapper->SetScalarModeToUsePointFieldData();
            tubeMapper->SelectColorArray("Colors");
            vtkSmartPointer<vtkPolyData> input1 = vtkSmartPointer<vtkPolyData>::New();
            input1->ShallowCopy(triangleFilter->GetOutput());
            
            
//            // Create the tree
//            vtkSmartPointer<vtkCellLocator> cellLocator =
//            vtkSmartPointer<vtkCellLocator>::New();
//            cellLocator->SetDataSet(tuber->GetOutput());
//            cellLocator->BuildLocator();
//            
////            printf("#># %f %f %f\n",ret_middle.x,ret_middle.y,ret_middle.z);
//            double testPoint[3] = {ret_middle.x, ret_middle.y, ret_middle.z};
//            
//            //Find the closest points to TestPoint
//            double closestPoint[3];//the coordinates of the closest point will be returned here
//            double closestPointDist2; //the squared distance to the closest point will be returned here
//            vtkIdType cellId; //the cell id of the cell containing the closest point will be returned here
//            int subId; //this is rarely used (in triangle strips only, I believe)
//            cellLocator->FindClosestPoint(testPoint, closestPoint, cellId, subId, closestPointDist2);
//            
//            std::cout << "Coordinates of closest point: " << closestPoint[0] << " " << closestPoint[1] << " " << closestPoint[2] << std::endl;
//            std::cout << "Squared distance to closest point: " << closestPointDist2 << std::endl;
//            std::cout << "CellId: " << cellId << std::endl;
            
            
            //
            //-----------------------------------
            // decide which STL to place
            //-----------------------------------
            //
            
            if ((NmStl>3) && (delx < 0.25) && (dely > 0.25))
            {
                appendFilter[3]->AddInputData(input1);
                appendFilter[3]->Update();
                tubePolyData->GetPointData()->AddArray(colors[3]);
            }
            else if ((NmStl>2) && delz < 1.0)
            {
                appendFilter[2]->AddInputData(input1);
                appendFilter[2]->Update();
                tubePolyData->GetPointData()->AddArray(colors[2]);
            }
            else if ((NmStl>1) && sz_vec >= 3.0)
            {
                appendFilter[1]->AddInputData(input1);
                appendFilter[1]->Update();
                tubePolyData->GetPointData()->AddArray(colors[1]);
            }
            else if ((NmStl>0) )
            {
                appendFilter[0]->AddInputData(input1);
                appendFilter[0]->Update();
                tubePolyData->GetPointData()->AddArray(colors[0]);
            }
            
            //
            //-----------------------------------
            //
            
            
            renderer->AddActor(tubeActor);
        }
        else if (cnt == -1)
        {
            added_splines = false;
            now_cells++;
//            printf("# empty line - one voronoi cell is complete\n");
        }
//        else
//            printf("# ignore - found 'draw_all_pipes' but no points had been added, probably start of next voronoi cell draw_pipes in csv");
        
        if (print_stat_cnt++ >= 100) {
            print_stat_cnt = 0;
            gettimeofday(&tv2, NULL);
            double secs_so_far = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
            int remaining_splines = total_splines - now_splines;
            double projTotalTime = secs_so_far * ((double)total_splines / (double)now_splines);
            double projRemainingTime = projTotalTime - secs_so_far;
            printf ("# STAT: %f seconds, %f remaining: %d / %d splines, %d / %d faces, %d / %d cells, min_radius %f\n",
                    secs_so_far, projRemainingTime,
                    now_splines, total_splines,
                    now_faces, total_faces,
                    now_cells, total_cells,
                    min_tube_radius);
        }
    }

    // input CSV file needs to end with a line containing just one number, so we call "draw_all_pipes()"
    if (added_splines)
    {
        fprintf(stderr,"ERROR: Your input CSV file is mis-formatted. You need to call draw all pipes!\n");
        fprintf(stderr,"ERROR: Edit your input CSV file and add a line at the end, with a single number, such as,\n");
        fprintf(stderr,"0\n");
        exit(1);
    }
    

    //
    //-----------------------------------
    // Output STL file(s)
    //-----------------------------------
    //
    
    // Remove any duplicate points and out STL
	// need #num STL
    vtkSmartPointer<vtkCleanPolyData> cleanFilter[NmStl_Arr];
    for (int i=0; i < NmStl; ++i) {
        cleanFilter[i] = vtkSmartPointer<vtkCleanPolyData>::New();
        cleanFilter[i]->SetInputConnection(appendFilter[i]->GetOutputPort());
        cleanFilter[i]->Update();
        
        // make STL output
        vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
        char new_str[512];
        sprintf(new_str,"%s-%d.stl",out_stl,i);
        stlWriter->SetFileName(new_str);
        stlWriter->SetFileTypeToBinary();
        stlWriter->SetInputConnection(cleanFilter[i]->GetOutputPort());
        stlWriter->Write();
    }
   
    
    gettimeofday(&tv2, NULL);
    printf ("# STAT: %f seconds: STL written\n",
            (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
    
#if CFG_DO_RENDER
    // go!
    renderer->SetBackground(.4, .5, .6);
    renderWindow->Render();
    renderWindowInteractor->Start();
#endif
    
    return EXIT_SUCCESS;
}
