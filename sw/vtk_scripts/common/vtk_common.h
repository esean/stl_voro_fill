//
#include "version.h"
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkLineSource.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkCenterOfMass.h>
#include <vtkPolyDataNormals.h>
#include <vtkOBBTree.h>
#include <vtkExtractEdges.h>
#include <vtkTriangleFilter.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkFeatureEdges.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkWarpScalar.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkMath.h>
#include <numeric>
#include <stdlib.h>
#include <sys/stat.h>   // stat, does_file_exist()

#define EXTRA_DEBUG 0 // 1=on

// TODO: add prototypes here with doc about args

#define BUFFER_SZ   (1024*8)

std::string CFG_FILE = "build_cfg.ini";

bool read_stl(vtkSmartPointer<vtkSTLReader> *reader, std::string inputFilename, bool clean_stl_before, bool bRemesh = false);
bool write_stl(std::string inputFilename, vtkPolyData* mp, std::string txt, bool clean_stl_after, bool bRemesh = false);


void shared_print_version() {
    printf("Version %s\n",APP_VERSION.c_str());
}
// trim from right
inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}
// check if file exists
inline bool does_file_exist (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}
void die(std::string failure_msg) {
    printf("\n\nFATAL ERROR:%s\n",failure_msg.c_str());
    exit(1);
}
void log(std::string in) {
#if EXTRA_DEBUG
    printf("\n####\n#### LOG: %% %s %% ####\n####\n",in.c_str());
#endif
}
//=========================================================
// Returns: 0 if successful
int popen_cmd(const char * cmd, char *retBuf) {
    
    FILE *pf;
    char lbuf[128];
    int cnt = 0;

    printf("# DEBUG:popen_cmd(%s)\n",cmd);
    if (!(pf = popen(cmd,"r"))) {
        fprintf(stderr, "ERROR: popen_cmd(%s): Could not open pipe for output\n",cmd);
        return 1;
    }
    
    // Grab data from process execution
    memset(retBuf,0,sizeof(*retBuf));
    while (true) {
        //
        memset(lbuf,0,sizeof(lbuf));
        if (!fgets(lbuf, sizeof(lbuf) , pf))
            break;
        strcat(retBuf,lbuf);
        if (++cnt > 1000) {
            fprintf(stderr,"WARNING: popen_cmd(%s): timeout!\n",cmd);
            break;
        }
    }
    // remove final \n in refBuf (.rstrip())
    for (int i=strlen(retBuf); i>=0; --i)
        if ((retBuf[i] == '\n') || (retBuf[i] == '\r'))
            retBuf[i] = '\0';
    
    int stat;
    int wstat = WEXITSTATUS(stat = pclose (pf));
    if (stat < 0 || (wstat != 0 && wstat != 128+SIGPIPE))
    {
        fprintf(stderr,"ERROR: popen_cmd(%s): Failed to close command stream, return code %d\n",cmd,wstat);
        fprintf(stderr,"ERROR TEXT:%s\n",retBuf);
        return wstat;
    }
    return 0;
}
std::string strip_newlines(std::string in_str) {
    std::string whitespaces (" \t\f\v\n\r");
    std::size_t found = in_str.find_last_not_of(whitespaces);
    if (found!=std::string::npos)
      in_str.erase(found+1);
    else
      in_str.clear();            // in_str is all whitespace
    return in_str;
}
// Returns: unique filename
std::string get_unique_filename() {
    std::string ret = "";
    std::string cmd = "mktemp /tmp/tmp.XXXXXXXXXX";
    char retBuf[BUFFER_SZ];
    int rc = popen_cmd(cmd.c_str(),retBuf);
    std::string resp(retBuf);
    resp = strip_newlines(resp);
    if (rc) {
        printf("ERROR: get_unique_filename() FAILED to execute\n");
    } else {
        ret = resp;
    }
    return ret;
}
// does not modify input STL
// Returns: 0 if successful
int get_stl_volume(std::string stl_fn, float *volume) {
    printf("# DEBUG: get_stl_volume(%s)\n",stl_fn.c_str());
    if ( ! does_file_exist(stl_fn)) {
        printf("ERROR: get_stl_volume(%s) but file does not exist!\n",stl_fn.c_str());
        return 1;
    }
    if (!volume) {
        printf("ERROR: get_stl_volume(%s) 2nd param NULL!\n",stl_fn.c_str());
        return 1;
    }
    *volume = 0.0;
    // VOLUME_ML=237.582063
    std::string cmd = "stl_volume.py " + stl_fn + " | grep ^VOLUME_ML= | cut -d= -f2";
    char retBuf[BUFFER_SZ];
    int rc = popen_cmd(cmd.c_str(),retBuf);
    if (rc) {
        printf("ERROR: get_stl_volume(%s) FAILED %d\n",stl_fn.c_str(),rc);
        return rc;
    }
    *volume = atof(retBuf);
    return rc;
}
// Returns: 0 if successful
int fix_and_correct_STL(std::string stl_fn, bool bRemeshFilter = false) {
    printf("# DEBUG: fix_and_correct_STL(%s,%d)\n",stl_fn.c_str(),bRemeshFilter);
    if ( ! does_file_exist(stl_fn)) {
        printf("ERROR: fix_and_correct_STL(%s) but file does not exist!\n",stl_fn.c_str());
        return 1;
    }
    char retBuf[BUFFER_SZ];
    int rc;
    // bRemeshFilter
    if (bRemeshFilter) {
        printf("ERROR: fix_and_correct_STL() CANNOT REMESH!\n");
        return 1;
    }
    std::string cmd = "fix_stl.sh " + stl_fn;
    //std::string cmd = "admesh --write-ascii-stl=" + stl_fn + " " + stl_fn;
    rc = popen_cmd(cmd.c_str(),retBuf);
    if (rc)
        printf("ERROR: fix_and_correct_STL(%s) FAILED %d\n",stl_fn.c_str(),rc);
    return rc;
}
// Reads from CFG_FILE
// returns 0 on success
int read_config_file_key(std::string key, std::string *val) {
    int rc = -1;
    std::string cfg_file = CFG_FILE;
    printf("# DEBUG: read_config_file_key(%s,%s)\n",cfg_file.c_str(),key.c_str());
    if ( ! does_file_exist(cfg_file)) {
        printf("ERROR: read_config_file_key(%s) but file %s does not exist!\n",key.c_str(),cfg_file.c_str());
        return -3;
    }
    std::string cmd = "readset_inicfg.py -f " + cfg_file + " -r " + key;
    char retBuf[BUFFER_SZ];
    rc = popen_cmd(cmd.c_str(),retBuf);
    if (rc) {
        printf("ERROR: read_config_file_key(%s,%s) FAILED %d\n",cfg_file.c_str(),key.c_str(),rc);
    }
    std::string val2(retBuf);
    *val = val2;
    printf("# DEBUG: read_config_file_key(%s) = %s returning %d\n",key.c_str(),(*val).c_str(),rc);
    return rc;
}

/////////
///////// Use DeepCopy() for return ??
/////////
// output STL written to 1st arg 'stl_fn1'
// Returns: 0 if successful
int combine_two_STL(std::string stl_fn1, std::string stl_fn2) {
    printf("# DEBUG: combine_two_STL(%s,%s)\n",stl_fn1.c_str(),stl_fn2.c_str());
    if ( ! does_file_exist(stl_fn1)) {
        printf("ERROR: combine_two_STL(%s) but file does not exist!\n",stl_fn1.c_str());
        return 1;
    }
    if ( ! does_file_exist(stl_fn2)) {
        printf("ERROR: combine_two_STL(%s) but file does not exist!\n",stl_fn2.c_str());
        return 1;
    }
    // todo: kind of silly to call out here to a VTK app when we could just do the work here...
    std::string cmd = "append_stl " + stl_fn1 + " " + stl_fn2;
    char retBuf[BUFFER_SZ];
    int rc = popen_cmd(cmd.c_str(),retBuf);
    if (rc) {
        printf("ERROR: combine_two_STL(%s,%s) FAILED %d\n",stl_fn1.c_str(),stl_fn2.c_str(),rc);
    }
    return rc;
}
// TODO: return vtkSmartPointer<vtkPolyData>
int combine_two_vtkPolyData(vtkSmartPointer<vtkPolyData> pd1, vtkSmartPointer<vtkPolyData> pd2, std::string *outputFilename) {
    printf("# DEBUG: combine_two_vtkPolyData\n");
    
    // write each polydata to STL, then combine_two_STL(1,2) to combine, read STL and return that in polyData
    std::string tmp_fn1 = get_unique_filename();
    std::string tmp_fn2 = get_unique_filename();
    if (!write_stl(tmp_fn1,pd1,"0",true)) return -1;
    if (!write_stl(tmp_fn2,pd2,"0",true)) return -1;
    tmp_fn1 += "-0.stl";
    tmp_fn2 += "-0.stl";
    int rc = combine_two_STL(tmp_fn1,tmp_fn2);
    if (rc)
        printf("ERROR: combine_two_vtkPolyData(%s,%s) FAILED %d\n",tmp_fn1.c_str(),tmp_fn2.c_str(),rc);  // todo: return the error code
    tmp_fn1 += "-union.stl";
    
    // todo: use DeepCopy() ?
    
    //    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    //    if (!read_stl(&reader,tmp_fn1,true)) {
    //        printf("# ERROR: combine_two_vtkPolyData() failed reading STL %s\n",tmp_fn1.c_str());
    //        rc = 1;
    //    }
    //
    //    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    //    polyData = reader->GetOutput();
    //    returnPoly = polyData;
    *outputFilename = tmp_fn1;
    return rc;
}
////============== TODO: IDENTICAL, combine ==================//////

/////////
///////// Use DeepCopy() for return ??
/////////



typedef struct {
    float x,y,z;
} aPt;

typedef struct {
    aPt min,max;
    double bounds[6];
    double center[3];       // todo: aPt
    double centerOfMass[3]; // todo: aPt
    float diameterbounds;   // sqrt(x^2+y^2+z^2)
    vtkIdType number_of_points;
    vtkIdType number_of_cells;
    vtkIdType number_of_polys;
    int number_of_regions;
} model_info;


// figure out the bounding box of input STL model
void get_polydata_info(model_info* info, vtkPolyData* mp, bool print_stats) {
//    double bounds[6];   // in mm
    mp->GetBounds(info->bounds);
    info->min.x = info->bounds[0];
    info->max.x = info->bounds[1];
    info->min.y = info->bounds[2];
    info->max.y = info->bounds[3];
    info->min.z = info->bounds[4];
    info->max.z = info->bounds[5];
    info->center[0] = (info->max.x-info->min.x)/2+info->min.x;
    info->center[1] = (info->max.y-info->min.y)/2+info->min.y;
    info->center[2] = (info->max.z-info->min.z)/2+info->min.z;
    double xmax = info->max.x-info->min.x;
    double ymax = info->max.y-info->min.y;
    double zmax = info->max.z-info->min.z;
    info->diameterbounds = std::sqrt(xmax*xmax+ymax*ymax+zmax*zmax);
    if (print_stats) printf("# MODEL: bounds:%f %f %f:diameter = %f\n",xmax,ymax,zmax,info->diameterbounds);
    info->number_of_points = mp->GetNumberOfPoints();
    info->number_of_cells = mp->GetNumberOfCells();
    info->number_of_polys = mp->GetNumberOfPolys();
    
    // Compute the center of mass
    vtkSmartPointer<vtkCenterOfMass> centerOfMassFilter = vtkSmartPointer<vtkCenterOfMass>::New();
#if VTK_MAJOR_VERSION <= 5
    centerOfMassFilter->SetInput(mp);
#else
    centerOfMassFilter->SetInputData(mp);
#endif
    centerOfMassFilter->SetUseScalarsAsWeights(false);
    centerOfMassFilter->Update();
    centerOfMassFilter->GetCenter(info->centerOfMass);
    
    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputData(mp); // reader->GetOutputPort());
    triangleFilter->Update();
    
    vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
    connectivityFilter->SetInputConnection(triangleFilter->GetOutputPort());
    connectivityFilter->SetExtractionModeToAllRegions();
    connectivityFilter->Update();
    // TODO: if multiple regions, combine using hull
    info->number_of_regions = connectivityFilter->GetNumberOfExtractedRegions();
    
    if (print_stats) {
        std::cout  << "# MODEL: xmin: " << info->min.x << " " << "xmax: " << info->max.x << std::endl
        << "# MODEL: ymin: " << info->min.y << " " << "ymax: " << info->max.y << std::endl
        << "# MODEL: zmin: " << info->min.z << " " << "zmax: " << info->max.z << std::endl;
        printf("# MODEL: center = (%f,%f,%f)\n",info->center[0],info->center[1],info->center[2]);
        printf("# MODEL: centerOfMass = (%f,%f,%f)\n",info->centerOfMass[0],info->centerOfMass[1],info->centerOfMass[2]);
        
        printf("# Number of points    = %lld\n",info->number_of_points);
        printf("# Number of triangles = %lld\n",info->number_of_cells);
        printf("# Number of polys = %lld\n",info->number_of_polys);
        printf("# Number of STL regions = %d\n",info->number_of_regions);
    }
}








bool is_stl_watertight(vtkSmartPointer<vtkPolyData> polydata) {
    
    // It should print "0" if your data is nice, well-defined closed surface.
    vtkSmartPointer<vtkFeatureEdges> edges = vtkSmartPointer<vtkFeatureEdges>::New();
#if VTK_MAJOR_VERSION <= 5
    edges->SetInputConnection(polydata->GetOutputPort());
#else
    edges->SetInputData(polydata);
#endif
    edges->FeatureEdgesOff();
    edges->NonManifoldEdgesOn();
    edges->BoundaryEdgesOn();
    edges->Update();
    //cout << edges->GetOutput()->GetNumberOfCells() << endl;
    return (edges->GetOutput()->GetNumberOfCells() == 0);
}










void addLine(vtkSmartPointer<vtkRenderer> *renderer, aPt pt1, aPt pt2, aPt color) {
    double p0[3] = {pt1.x,pt1.y,pt1.z};
    double p1[3] = {pt2.x,pt2.y,pt2.z};
    
    vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
    lineSource->SetPoint1(p0);
    lineSource->SetPoint2(p1);
    lineSource->Update();
    
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(lineSource->GetOutputPort());
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(color.x,color.y,color.z);
//    actor->GetProperty()->SetLineWidth(4);
    (*renderer)->AddActor(actor);
}
void addPoint(vtkSmartPointer<vtkRenderer> *renderer, aPt p, float radius, aPt color) {
    //Create a sphere
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->Update();
    double cntr[3] = {p.x,p.y,p.z};
    sphereSource->SetCenter(cntr);
    sphereSource->SetRadius(radius);
    sphereSource->SetPhiResolution(100);
    sphereSource->SetThetaResolution(100);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphereSource->GetOutputPort());
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(color.x,color.y,color.z);
    (*renderer)->AddActor(actor);
}
double distance(double x,double y,double z) {
    return std::sqrt(x*x+y*y+z*z);
}
double distance_two_points(double p0[3], double p1[3]) {
    return distance(p0[0]-p1[0],p0[1]-p1[1],p0[2]-p1[2]);
}


void take_screenshot_renderer(vtkSmartPointer<vtkRenderWindow> *vtkRenderWindow, std::string out_fn, int magnification) {
    // Screenshot
    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    windowToImageFilter->SetInput(*vtkRenderWindow);
    windowToImageFilter->SetMagnification(magnification); //set the resolution of the output image (3 times the current resolution of vtk render window)
//        windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
//    windowToImageFilter->ReadFrontBufferOff(); // read from the back buffer
    windowToImageFilter->FixBoundaryOn();
    windowToImageFilter->Update();
    
    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
    writer->SetFileName(out_fn.c_str());
    writer->SetCompressionLevel(0); // store best image, no compression
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
    printf("# SCREENSHOT:%s\n",out_fn.c_str());
}


double find_angle_degrees_between_two_vectors(aPt vec1, aPt vec2) {
    log("find_angle_degrees_between_two_vectors");
    double a[3] = {vec1.x,vec1.y,vec1.z};
    double b[3] = {vec2.x,vec2.y,vec2.z};
    double angle_rad = vtkMath::AngleBetweenVectors(a,b);
    return vtkMath::DegreesFromRadians(angle_rad);
}

//double find_dot_product(aPt vec1, aPt vec2) {
//    double a[3] = {vec1.x,vec1.y,vec1.z};
//    double b[3] = {vec2.x,vec2.y,vec2.z};
//    double dot_product = vtkMath::Dot(a,b);
//    return dot_product;
//}
//double find_magnitude_of_vector(aPt vec) {
//    double mag = std::sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
//    return mag;
//}
//
//double find_distance_between_two_points(aPt base, aPt end_arrow) {
//    // Create two points.
//    double p0[3] = {base.x, base.y, base.z};
//    double p1[3] = {end_arrow.x, end_arrow.y, end_arrow.z};
//    
//    // Find the squared distance between the points.
//    double squaredDistance = vtkMath::Distance2BetweenPoints(p0, p1);
//    
//    // Take the square root to get the Euclidean distance between the points.
//    double distance = std::sqrt(squaredDistance);
//    
//    return distance;
//}

// return a vector, starting at base, pointing toward end_arrow
// formula: return vec = end_arrow - base
aPt make_vector_from_point_to_point(aPt base, aPt end_arrow) {
    aPt vec = {end_arrow.x-base.x, end_arrow.y-base.y, end_arrow.z-base.z};
    return vec;
}

// make ptN vector to a unit vector
aPt make_vector_2_unit_vector(aPt vec) {
    //
    double vec_mag = std::sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
    aPt Unit_vec = {vec.x/vec_mag, vec.y/vec_mag, vec.z/vec_mag};
    return Unit_vec;
}

// make a vector from 'orig' pointing out along 'normal', converted
// to unit vector, then extending out scaled by 'mult'
aPt make_vector_along_normal(aPt orig, aPt normal, aPt mult) {
    //
    aPt Unit_vecN = make_vector_2_unit_vector(normal);
    aPt alongPt = {orig.x+Unit_vecN.x*mult.x,
        orig.y+Unit_vecN.y*mult.y, orig.z+Unit_vecN.z*mult.z};
    return alongPt;
}

// do 'ray-casting' to find all intersection points with a line and an vtkOBBTree surface
// assumes you have already done this,
//     vtkSmartPointer<vtkOBBTree> tree = vtkSmartPointer<vtkOBBTree>::New();
//     tree->SetDataSet(polyData);
//     tree->BuildLocator();
//
vtkSmartPointer<vtkPoints> find_intersection_points_line_and_surface(aPt orig, aPt normal, aPt normal_mult, vtkSmartPointer<vtkOBBTree> tree) {
    //
    aPt alongPt = make_vector_along_normal(orig,normal,normal_mult);
    double pI1[3] = {orig.x,orig.y,orig.z};
    double pI2[3] = {alongPt.x,alongPt.y,alongPt.z};
    vtkSmartPointer<vtkPoints> interPts = vtkSmartPointer<vtkPoints>::New();
    int ptsFound = tree->IntersectWithLine(pI1,pI2,interPts,NULL);
    //std::cout << "Found intersection points:" << ptsFound << std::endl;
    return interPts;
}

//// Assumes these have already been done,
//
//     // NOTE: faster if normals pre-computed with,
//     vtkSmartPointer<vtkFloatArray> normals = vtkSmartPointer<vtkFloatArray>::New();
//     GetPointNormals(&reader,&normals,false);
//     if (!normals) {
//         fprintf(stderr,"No point normals were found!\n");
//         return EXIT_FAILURE;
//     }
//     inner_stl->GetPointData()->SetNormals(inner_normals);
//
// idx is point index, mult scales unit normal extending from polyData[idx]
//
// and tree is from,
//     // Create the locator
//     vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
//     polyData = reader->GetOutput();
//     vtkSmartPointer<vtkOBBTree> tree = vtkSmartPointer<vtkOBBTree>::New();
//     tree->SetDataSet(polyData);
//     tree->BuildLocator();
//
// this effectively is finding the min distance thru an STL surface: min(point normal inside distance to other side), and returning the
//  location of the point on other side where it found that min distance
//
// polyData - contains the point @ [idx]
// normals - unit normal of the point
// mult - amount needed to shoot normal line thru model (based on bounds)
// tree - OBBTree of the surface
// idx - index of the point
// min_intersect_pt - (RETURN) point on the surface found at min distance
//
float find_min_distance_polydata_inner_surface(aPt pt, aPt vecN, aPt mult, vtkSmartPointer<vtkOBBTree> tree, aPt *min_intersect_pt) {
    //
    float this_min_dist = 1e+9;
    double p[3] = {pt.x,pt.y,pt.z};
    
    // extend out to xmax a line from p[] along point normal direction vector normals[]
    vtkSmartPointer<vtkPoints> interPts = find_intersection_points_line_and_surface(pt,vecN,mult,tree);
    vtkIdType numPts = interPts->GetNumberOfPoints();
    
    for (vtkIdType pointId=0; pointId < numPts; pointId++ )
    {
        double x[3];
        interPts->GetPoint(pointId, x);
        double dist = distance_two_points(x,p);
        
        if (dist > 0) {
//            printf("dist=%f\n",dist);
            if (dist < this_min_dist) {
                this_min_dist = dist;
                aPt tmp = {x[0],x[1],x[2]};
                if (min_intersect_pt) *min_intersect_pt = tmp;
            }
        }
    }
    return (this_min_dist == 1e+9) ? 0.0 : this_min_dist;
}

//// Assumes these have already been done,
//
//    // NOTE: faster if normals pre-computed with,
//    vtkSmartPointer<vtkFloatArray> normals = vtkSmartPointer<vtkFloatArray>::New();
//    GetPointNormals(&reader,&normals,false);
//    if (!normals) {
//        fprintf(stderr,"No point normals were found!\n");
//        return EXIT_FAILURE;
//    }
//    inner_stl->GetPointData()->SetNormals(inner_normals);
//    vtkSmartPointer<vtkPolyData> inner_polydata_surface = vtkSmartPointer<vtkPolyData>::New();
//    inner_polydata_surface = reader->GetOutput();
//    vtkSmartPointer<vtkImplicitPolyDataDistance> implicitPolyDataDistance = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
//    implicitPolyDataDistance->SetInput(inner_polydata_surface);
//
// this effectively is finding the distance from outer STL surface to the closest point on the inner STL, and returning the
//  location of that closest point on inner surface
//
// polyData - contains the outer surface point
// normals - normals of the outer surface point
// idx - index of the outer surface point
// implicitPolyDataDistance - as says, init with 'inner_polydata_surface'
// min_intersect_pt - (RETURN) closest point on the inner surface
//
float find_min_distance_point_2_polydata(aPt pt, aPt vecN, vtkSmartPointer<vtkImplicitPolyDataDistance> implicitPolyDataDistance, aPt *min_intersect_pt) {
    //
    double p[3] = {pt.x,pt.y,pt.z};

    // we want the closest distance from a point (on outer surface) to the inner surface
    double x[3];
    double signedDistance = implicitPolyDataDistance->EvaluateFunctionAndGetClosestPoint(p, x);
    aPt min_intsc = {x[0],x[1],x[2]};
    if (min_intersect_pt) *min_intersect_pt = min_intsc;
    
    // 052317sbh: implicitPolyDataDistance() can sometimes return negative distance when they shouldn't be, so check if dist<0.
    //      We know the outer surface point normal vector, and we can compute the vector from outside surface to closest point in inside surface,
    //      so find the angle between these two. If it's generally pointing in same direction, then it's not a negative distance.
    if (signedDistance < 0.0) {

        // this is vector: pt (outer surface) pointing at "closest point" pt on inner surface
        aPt vec_pointing_inside = make_vector_from_point_to_point(pt,min_intsc);
        // so we know the point normal, and our "closest-point vector", so we can use dot product to find the
        // angle between these two vectors, if it points in same direction, the distance is not negative
        double ang_deg = find_angle_degrees_between_two_vectors(vecN,vec_pointing_inside);

        if (std::abs(ang_deg) < 90.0) {
#if EXTRA_DEBUG
            printf("# WARNING:find_min_distance_point_2_polydata(): found negative distance outer to inner, but ray is in same direction (%f deg) as point normal vector...\n",ang_deg);
#endif
            // invert distance since we now think it's ok to use
            signedDistance *= -1.0f;
        } else {
        	printf("# WARNING:negative distance,angle =%f,%f -\nWARNING: either this is really negative, or your shape could have inverted normals in places. Fix first\n",signedDistance,ang_deg);
	}
    }
    
    return (float)signedDistance;
}





enum mOperation { DIFFERENCE, UNION, INTERSECTION };

vtkSmartPointer<vtkPolyData> perform_boolean_operation(vtkSmartPointer<vtkPolyData> stl1, vtkSmartPointer<vtkPolyData> stl2, mOperation op) {
    //
    printf("perform_boolean_operation: op=%d\n",(int)op);
    
    vtkSmartPointer<vtkBooleanOperationPolyDataFilter> booleanOperation = vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();
    if (op == UNION) {
	// instead, 'admesh --merge=name'
        // todo 0721: use append_stl instead, MUCH faster!
        printf("\nWARNING: vtkSmartPointer<vtkPolyData> perform_boolean_operation(UNION) recommends you use combine_two_STL() instead\n\n");
        booleanOperation->SetOperationToUnion();
    }
    else if (op == INTERSECTION)
        booleanOperation->SetOperationToIntersection();
    else
        booleanOperation->SetOperationToDifference();
    
#if VTK_MAJOR_VERSION <= 5
    booleanOperation->SetInputConnection( 0, stl1->GetProducerPort() );
    booleanOperation->SetInputConnection( 1, stl2->GetProducerPort() );
#else
    booleanOperation->SetInputData( 0, stl1 );
    booleanOperation->SetInputData( 1, stl2 );
#endif
    printf("PERFORM: pre-update\n");
    booleanOperation->Update();
    printf("PERFORM: after-update\n");
    
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData = booleanOperation->GetOutput();
    
    return polyData;
}





// TODO: should be smartptr?
bool GetPointNormalsInfo(vtkSmartPointer<vtkPolyData> polydata)
{
#if EXTRA_DEBUG
    std::cout << "# FYI - In GetPointNormalsInfo: pts=" << polydata->GetNumberOfPoints() << std::endl;
    std::cout << "# FYI - Looking for point normals..." << std::endl;
#endif
    
    //    // Count points
    //    vtkIdType numPoints = polydata->GetNumberOfPoints();
    //    std::cout << "# There are " << numPoints << " points." << std::endl;
    //
    //    // Count triangles
    //    vtkIdType numPolys = polydata->GetNumberOfPolys();
    //    std::cout << "# There are " << numPolys << " polys." << std::endl;
    
    ////////////////////////////////////////////////////////////////
    // Double normals in an array
    vtkDoubleArray* normalDataDouble =
    vtkDoubleArray::SafeDownCast(polydata->GetPointData()->GetArray("Normals"));
    
    if(normalDataDouble)
    {
#if EXTRA_DEBUG
        int nc = normalDataDouble->GetNumberOfTuples();
        std::cout << "# FYI - There are " << nc
        << " components in normalDataDouble" << std::endl;
#endif
        return true;
    }
    
    ////////////////////////////////////////////////////////////////
    // Double normals in an array
    vtkFloatArray* normalDataFloat =
    vtkFloatArray::SafeDownCast(polydata->GetPointData()->GetArray("Normals"));
    
    if(normalDataFloat)
    {
#if EXTRA_DEBUG
        int nc = normalDataFloat->GetNumberOfTuples();
        std::cout << "# FYI - There are " << nc
        << " components in normalDataFloat" << std::endl;
#endif
        return true;
    }
    
    ////////////////////////////////////////////////////////////////
    // Point normals
    vtkDoubleArray* normalsDouble =
    vtkDoubleArray::SafeDownCast(polydata->GetPointData()->GetNormals());
    
    if(normalsDouble)
    {
#if EXTRA_DEBUG
        std::cout << "# FYI - There are " << normalsDouble->GetNumberOfComponents()
        << " components in normalsDouble" << std::endl;
#endif
        return true;
    }
    
    ////////////////////////////////////////////////////////////////
    // Point normals
    vtkFloatArray* normalsFloat =
    vtkFloatArray::SafeDownCast(polydata->GetPointData()->GetNormals());
    
    if(normalsFloat)
    {
#if EXTRA_DEBUG
        std::cout << "# FYI - There are " << normalsFloat->GetNumberOfComponents()
        << " components in normalsFloat" << std::endl;
#endif
        return true;
    }
    
    /////////////////////////////////////////////////////////////////////
    // Generic type point normals
    vtkDataArray* normalsGeneric = polydata->GetPointData()->GetNormals(); //works
    if(normalsGeneric)
    {
#if EXTRA_DEBUG
        std::cout << "# FYI - There are " << normalsGeneric->GetNumberOfTuples()
        << " normals in normalsGeneric" << std::endl;
        
        double testDouble[3];
        normalsGeneric->GetTuple(0, testDouble);
        
        std::cout << "Double: " << testDouble[0] << " "
        << testDouble[1] << " " << testDouble[2] << std::endl;
        
        // Can't do this:
        /*
         float testFloat[3];
         normalsGeneric->GetTuple(0, testFloat);
         
         std::cout << "Float: " << testFloat[0] << " "
         << testFloat[1] << " " << testFloat[2] << std::endl;
         */
#endif
        return true;
    }
    
    
    // If the function has not yet quit, there were none of these types of normals
    std::cout << "# WARNING: FYI - Normals not found!" << std::endl;
    return false;
    
}

// easy todo: add inner_stl->GetPointData()->SetNormals(inner_normals); here, then return that polydata
void GetPointNormals(vtkSmartPointer<vtkPolyData> polydata, vtkSmartPointer<vtkFloatArray> *normals, bool normals_face_out) {

    log("GetPointNormals");
    if (!GetPointNormalsInfo(polydata))
        printf("ERROR: FYI - vtk_common::GetPointNormals():GetPointNormalsInfo() returned false\n");
    
    // Generate normals
    vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
#if VTK_MAJOR_VERSION <= 5
    normalGenerator->SetInput(polydata);
#else
    normalGenerator->SetInputData(polydata);
#endif
    normalGenerator->ComputePointNormalsOn();           // # Ensable normal calculation at cell vertices
    normalGenerator->ComputeCellNormalsOff();           // # Disable normal calculation at cell centers
    normalGenerator->SetFeatureAngle(0.1);
    normalGenerator->SetSplitting(1);
//    normalGenerator->SetConsistency(0);
//    normalGenerator->SplittingOff();                // # Disable splitting of sharp edges
    if (normals_face_out)
        normalGenerator->FlipNormalsOff();
    else
        normalGenerator->FlipNormalsOn();           // # Enable global flipping of normal orientation
    normalGenerator->ConsistencyOn();
    normalGenerator->AutoOrientNormalsOn();     // # Enable automatic determination of correct normal orientation
    normalGenerator->Update();          // # Perform calculation
    
    vtkSmartPointer<vtkPolyData> polyDataNormals = vtkSmartPointer<vtkPolyData>::New();
    polyDataNormals = normalGenerator->GetOutput();
    //    bool ok = GetPointNormalsInfo(polyDataNormals); // just fyi, can comment out
    ///////// Get Point Normals ///////////
    *normals = vtkFloatArray::SafeDownCast(polyDataNormals->GetPointData()->GetArray("Normals"));
    
    polydata->GetPointData()->SetNormals(*normals);
    if (!GetPointNormalsInfo(polydata))
        printf("ERROR: FYI - vtk_common::GetPointNormals():GetPointNormalsInfo() returned false\n");
#if EXTRA_DEBUG
    printf("\\-------\n");
#endif
}









// bRemesh only applies if clean_stl_before is true
bool read_stl(vtkSmartPointer<vtkSTLReader> *reader, std::string inputFilename, bool clean_stl_before, bool bRemesh) {
    
    printf("# ++ DEBUG: read_stl(%s, remesh=%d) START\n",inputFilename.c_str(),bRemesh);
    if ( ! does_file_exist(inputFilename)) {
        printf("ERROR: read_stl(%s) but file does not exist!\n",inputFilename.c_str());
        return false;
    }
    // fix any STL before we open them
    if (clean_stl_before) {
        printf("# ++ DEBUG: read_stl(%s) CLEAN 1st\n",inputFilename.c_str());
        if (fix_and_correct_STL(inputFilename,bRemesh)) {
            printf("ERROR: read_stl(%s) failed call to fix_and_correct_STL()\n",inputFilename.c_str());
            return false;
        }
    }
    (*reader)->SetFileName(inputFilename.c_str());
    (*reader)->Update();
    printf("# ++ DEBUG: read_stl(%s) SUCCESS\n",inputFilename.c_str());
    return true;
}

// bRemesh only applies if clean_stl_before is true
bool write_stl(std::string inputFilename, vtkPolyData* mp, std::string txt, bool clean_stl_after, bool bRemesh) {
    // todo: need to triangle filter first?
    //Append meshes
    printf("# === DEBUG: write_stl(%s-%s, remesh=%d) START\n",inputFilename.c_str(),txt.c_str(),bRemesh);
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
#if VTK_MAJOR_VERSION <= 5
    appendFilter->AddInputConnection(mp->GetProducerPort());
#else
    appendFilter->AddInputData(mp);
#endif
    appendFilter->Update();
    vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanFilter->SetInputConnection(appendFilter->GetOutputPort());
    cleanFilter->Update();
    
    // make STL output
    vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
    char new_str[512];
    if (!txt.empty())
        sprintf(new_str,"%s-%s.stl",inputFilename.c_str(),txt.c_str());
    else
        sprintf(new_str,"%s",inputFilename.c_str());
    stlWriter->SetFileName(new_str);
    stlWriter->SetFileTypeToBinary();
    stlWriter->SetInputConnection(cleanFilter->GetOutputPort());
    printf("# INFO:write_stl() writing output STL:%s\n",new_str);
    stlWriter->Write();
    
    if ( ! does_file_exist(new_str)) {
        printf("ERROR: write_stl(%s) wrote file, but file does not exist!\n",new_str);
        return false;
    }
    
    // fix any STL after we write them
    if (clean_stl_after) {
        printf("# === DEBUG: write_stl(%s) CLEAN after\n",new_str);
        std::string new_fn(new_str);
        if (fix_and_correct_STL(new_fn,bRemesh)) {
            printf("ERROR: write_stl(%s) failed call to fix_and_correct_STL()\n",new_str);
            return false;
        }
    }
    printf("# === DEBUG: write_stl(%s) SUCCESS\n",new_str);
    return true;
}








// from http://www.vtk.org/Wiki/VTK/Examples/Cxx/PolyData/VertexConnectivity
//
// run before, or be using a polygon data source (STLreader)
//      vtkSmartPointer<vtkExtractEdges> extractEdges = vtkSmartPointer<vtkExtractEdges>::New();
//      extractEdges->SetInputConnection(triangleFilter->GetOutputPort());
//      extractEdges->Update();
//      vtkSmartPointer<vtkPolyData> mesh = extractEdges->GetOutput();
//      vtkSmartPointer<vtkIdList> connectedVertices = GetConnectedVertices(mesh, 0);
//
vtkSmartPointer<vtkIdList> GetConnectedVertices(vtkSmartPointer<vtkPolyData> mesh, int id)
{
    vtkSmartPointer<vtkIdList> connectedVertices = vtkSmartPointer<vtkIdList>::New();
    
    //get all cells that vertex 'id' is a part of
    vtkSmartPointer<vtkIdList> cellIdList = vtkSmartPointer<vtkIdList>::New();
    mesh->GetPointCells(id, cellIdList);
    
//    cout << "Vertex 0 is used in cells ";
//    for(vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++)
//    {
//        cout << cellIdList->GetId(i) << ", ";
//    }
//    cout << endl;
   
    for(vtkIdType i = 0; i < cellIdList->GetNumberOfIds(); i++)
    {
//        cout << "id " << i << " : " << cellIdList->GetId(i) << endl;
        
        vtkSmartPointer<vtkIdList> pointIdList = vtkSmartPointer<vtkIdList>::New();
        mesh->GetCellPoints(cellIdList->GetId(i), pointIdList);
        
//        cout << "End points are " << pointIdList->GetId(0) << " and " << pointIdList->GetId(1) << endl;
        
        if(pointIdList->GetId(0) != id)
        {
//            cout << "Connected to " << pointIdList->GetId(0) << endl;
            connectedVertices->InsertNextId(pointIdList->GetId(0));
        }
        else
        {
//            cout << "Connected to " << pointIdList->GetId(1) << endl;
            connectedVertices->InsertNextId(pointIdList->GetId(1));
        }
    }
    
    return connectedVertices;
}


//void TestPointNormals(vtkSmartPointer<vtkPolyData> polydata)
//{
//    std::cout << "In TestPointNormals: " << polydata->GetNumberOfPoints() << std::endl;
//    // Try to read normals directly
//    vtkSmartPointer<vtkFloatArray> normals = vtkSmartPointer<vtkFloatArray>::New();
//    GetPointNormals(polydata,&normals,false);
//    bool hasPointNormals = normals;
//    inner_stl->GetPointData()->SetNormals(inner_normals);
//
//    if(!hasPointNormals)
//    {
//        std::cout << "No point normals were found. Computing normals..." << std::endl;
//    }
//}


// assumes you have already done this,
//     vtkSmartPointer<vtkOBBTree> tree = vtkSmartPointer<vtkOBBTree>::New();
//     tree->SetDataSet(polyData);
//     tree->BuildLocator();

// pass model_info for bounds min/max
// NOTE: set bsmooth = true when intruding (scale_factor < 0), and set = false if scale_factor > 0
vtkSmartPointer<vtkPolyData> extrude_polydata_along_normal(vtkSmartPointer<vtkPolyData> stl_outer_shell, float scale_factor, vtkSmartPointer<vtkOBBTree> tree, float min_channel, bool bsmooth, model_info info, int *returnCode, bool bIntrudeClosestPtVector, vtkSmartPointer<vtkPolyData> stl_inner) {
    
    log("extrude_polydata_along_normal");
    if (returnCode)
        *returnCode = -1;
    
    // return polydata
    vtkSmartPointer<vtkPolyData> out_poly = vtkSmartPointer<vtkPolyData>::New();
    int numberOfPoints = stl_outer_shell->GetNumberOfPoints();
    
    ///////// Get Point Normals ///////////
    vtkSmartPointer<vtkFloatArray> normals = vtkSmartPointer<vtkFloatArray>::New();
    GetPointNormals(stl_outer_shell,&normals,false);
    if (!normals) {
        fprintf(stderr,"No point normals were found!\n");
        return out_poly;
    }
#if EXTRA_DEBUG
    std::cout << "# There are " << normals->GetNumberOfTuples() << " point normals." << std::endl;
#endif
    stl_outer_shell->GetPointData()->SetNormals(normals);
    
    // Create Scalars
    vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
    scalars->SetNumberOfTuples(numberOfPoints);
    // "closest pt" normals
    vtkSmartPointer<vtkFloatArray> closestPt_normals = vtkSmartPointer<vtkFloatArray>::New();
    closestPt_normals->SetNumberOfComponents(3); //3d normals (ie x,y,z)
    closestPt_normals->SetNumberOfTuples(numberOfPoints);
    
    // init the 2nd (innter) polydata so we can compute signed distances below, makes it *much* faster
    vtkSmartPointer<vtkImplicitPolyDataDistance> implicitPolyDataDistance = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    if (bIntrudeClosestPtVector)
    	implicitPolyDataDistance->SetInput(stl_inner);
    
    // Go thru all points, and set the scale based on distance to opposing cell.
    // "scale" really is mm either added or subtracted, by each point normal.
    // if you look at opposing point normal (other insider of surface) and set the
    // scale based on both distances to as to maintain a minimum channel thru surface
    
    // want the scaling such that if both sides were pulled in, a minimum channel would be left
    // remember, scale_factor is *really* just 'mm' taken off, or added
    // scale_factor is the max you should ever extrude, the min is min_channel
    
    double xmax = info.max.x-info.min.x;
    double ymax = info.max.y-info.min.y;
    double zmax = info.max.z-info.min.z;
    
    aPt min_intersect_pt;
    aPt mult = {xmax*1.5,ymax*1.5,zmax*1.5};
    float min_thick_mm = std::abs(scale_factor)*2 + min_channel;
    bool bScaleNegative = scale_factor < 0.0;
    
    for(vtkIdType i = 0; i < numberOfPoints; ++i)
    {
        float sx = scale_factor;
        
        //
        // if we are extruding inward, need to process some more
        //
        if (sx < 0) {
            // dist is the closest of all intersection points
            double p[3];
            stl_outer_shell->GetPoint(i, p);
            aPt pt = {p[0],p[1],p[2]};
            double ptN[3];
            normals->GetTuple(i,ptN);
            aPt vecN = {ptN[0],ptN[1],ptN[2]};
            float dist;
            
            
            if (bIntrudeClosestPtVector) {
                /// find distance from 'pt' to closest point on 'stl_inner'
                dist = find_min_distance_point_2_polydata(pt,vecN,implicitPolyDataDistance,&min_intersect_pt);
//                printf("in_bdist_out2in=%f\n",dist);
                
                // set closest point normal, normal facing out since we intruding with negative mm
                aPt closestPtN = make_vector_2_unit_vector(make_vector_from_point_to_point(min_intersect_pt,pt));
                // and make one facing in for next distance calc
                vecN = make_vector_2_unit_vector(make_vector_from_point_to_point(pt,min_intersect_pt));
                double closestPt_vec[3] = {closestPtN.x, closestPtN.y, closestPtN.z};
                closestPt_normals->SetTuple(i, closestPt_vec);
                
                // now find distance thru inner surface along closest pt vecN
            }
            
            dist = find_min_distance_polydata_inner_surface(pt,vecN,mult,tree,&min_intersect_pt);

            //printf("sx=%f min=%f\n",sx,min_thick_mm);
            if (dist < min_thick_mm) {
                sx = (dist - min_channel)/2;
                
                if (sx < 0) {
#if EXTRA_DEBUG
                    printf("WARNING: xyz %f %f %f:point[%lld/%d] dist=%f does not allow extrusion %f mm and keep minimum channel %f mm, no extrusion performed\n",
                           p[0],p[1],p[2],i,numberOfPoints,dist,sx,min_channel);
#endif
                    sx = 0;
                }
                if (bScaleNegative)
                    sx *= -1.0;
            }
        }
        
        
        // set value
        scalars->SetTuple1(i,sx);
    }
    stl_outer_shell->GetPointData()->SetScalars(scalars);
    
    vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    vtkSmartPointer<vtkWarpScalar> warp = vtkSmartPointer<vtkWarpScalar>::New();
    
    if (bIntrudeClosestPtVector) {
        // closest point vector
        stl_outer_shell->GetPointData()->SetNormals(closestPt_normals);
        warp->SetInputData(stl_outer_shell);
        
//        if (!GetPointNormalsInfo(stl_outer_shell))
//            printf("ERROR: FYI - vtk_common::extrude_polydata_along_normal():GetPointNormalsInfo(stl_outer_shell) returned false\n");
    }
    else {
        // point normal vector 
        //
    #if VTK_MAJOR_VERSION <= 5
        normalGenerator->SetInput(stl_outer_shell);
    #else
        normalGenerator->SetInputData(stl_outer_shell);
    #endif
        normalGenerator->SplittingOff();
        normalGenerator->FlipNormalsOff();
        normalGenerator->ComputePointNormalsOn();
        normalGenerator->ComputeCellNormalsOff();
        normalGenerator->Update();
        warp->SetInputConnection(normalGenerator->GetOutputPort());
    }
    
    // Required for SetNormal to have an effect
    warp->UseNormalOff();
    warp->SetScaleFactor(1); // use the scalars themselves
    warp->Update();
    
    if (bsmooth) {
        vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
        smoother->SetInputConnection(warp->GetOutputPort());
        smoother->SetNumberOfIterations(15);    // todo: increasing may cause a skeleton effect
        smoother->BoundarySmoothingOff();
        smoother->FeatureEdgeSmoothingOff();
        smoother->SetFeatureAngle(120.0);
        smoother->SetPassBand(0.001);
        smoother->NonManifoldSmoothingOn();
        smoother->NormalizeCoordinatesOn();
        smoother->Update();
        
        // warp is vtkPointSet, so inherit to vtkPolyData
        out_poly->DeepCopy(smoother->GetOutput());
        
        if (bIntrudeClosestPtVector)
            // closest point vector
            out_poly->GetPointData()->SetNormals(closestPt_normals);
    } else {
        
        // warp is vtkPointSet, so inherit to vtkPolyData
        out_poly->DeepCopy(warp->GetOutput());
    }
    
    *returnCode = 0;
    return out_poly;
}


vtkSmartPointer<vtkPolyData> reset_polydata_info_by_writing_reading_STL(vtkSmartPointer<vtkPolyData> in) {
    // write 'in' to STL, then return what you read, only need to clean on read
    
    log("reset_polydata_info_by_writing_reading_STL");
    
    std::string tmp_fn1 = get_unique_filename();
    vtkSmartPointer<vtkSTLReader> reader3 = vtkSmartPointer<vtkSTLReader>::New();
    vtkSmartPointer<vtkPolyData> fixed_stl = vtkSmartPointer<vtkPolyData>::New();
    
    if (!write_stl(tmp_fn1,in,"",true))
        return fixed_stl;
    if (!read_stl(&reader3,tmp_fn1,true))
        return fixed_stl;
    fixed_stl = reader3->GetOutput();
    return fixed_stl;
}


// takes 2 polydata, returns clearance info
typedef struct  {
    int times_in_negative_distance;  // indicates models are touching
    int times_in_range;             // distance fell between user supplied min_mm & max_mm
    double mean_mm;
    double stddev_mm;
    double min_dist;
} clearance_info;

bool compute_basic_clearance(vtkSmartPointer<vtkPolyData> outer_stl, vtkSmartPointer<vtkPolyData> inner_stl, float min_mm, float max_mm, clearance_info *cl_info) {
    
    log("compute_basic_clearance");

    // find basic model info
    model_info info;
    get_polydata_info(&info,outer_stl,false);
    double xmax = info.max.x-info.min.x;
    double ymax = info.max.y-info.min.y;
    double zmax = info.max.z-info.min.z;
    double min_dist = info.diameterbounds;    // max
    long long int numPts = info.number_of_points;
    
    vtkSmartPointer<vtkFloatArray> inner_normals = vtkSmartPointer<vtkFloatArray>::New();
    GetPointNormals(inner_stl,&inner_normals,false);
    if (!inner_normals) {
        fprintf(stderr,"No point normals were found!\n");
        return EXIT_FAILURE;
    }
#if EXTRA_DEBUG
    std::cout << "# There are " << inner_normals->GetNumberOfTuples() << " inner point normals." << std::endl;
#endif
    // todo: compute_min_inner doesn't have it here...
// SBH
//    inner_stl->GetPointData()->SetNormals(inner_normals);
    
    vtkSmartPointer<vtkFloatArray> outer_normals = vtkSmartPointer<vtkFloatArray>::New();
    GetPointNormals(outer_stl,&outer_normals,false);
    if (!outer_normals) {
        fprintf(stderr,"No point outer_normals were found!\n");
        return EXIT_FAILURE;
    }
#if EXTRA_DEBUG
    std::cout << "# There are " << outer_normals->GetNumberOfTuples() << " outer point normals." << std::endl;
#endif
    outer_stl->GetPointData()->SetNormals(outer_normals);
    
    long long int percent = numPts/100;
    if (percent == 0) percent = numPts/2;
    if (percent == 0) percent = 1;
    
    // for each point in polydata, draw a line along it's normal vector
    // to find intersection with surface. Compute distance from those
    // points.
    // Write all of the coordinates of the points in the vtkPolyData to the console.
    
    // store all found min_dist for each point
    double min_dist_clr[numPts];
    aPt mult = {xmax*1.5,ymax*1.5,zmax*1.5};
    std::vector<double> v;
    
    // todo
    // init the 2nd (innter) polydata so we can compute signed distances below, makes it *much* faster
    vtkSmartPointer<vtkImplicitPolyDataDistance> implicitPolyDataDistance = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    implicitPolyDataDistance->SetInput(inner_stl);
    
    int num_times_negative_distance = 0;
    int num_times_in_user_range = 0;
    
    for(vtkIdType idx = 0; idx < numPts; idx++)
    {
#if EXTRA_DEBUG
        if (idx%percent == 0)
            printf("Pts %lld out of %lld\n",idx,numPts);
#endif
        
        // get point on outside surface
        double p[3];
        outer_stl->GetPoint(idx,p);
        aPt pt = {p[0],p[1],p[2]};
        double pN[3];
        outer_normals->GetTuple(idx,pN);
        aPt ptN = {pN[0],pN[1],pN[2]};
        
        // for each intersection point found, keep a min_dist
        double this_min_dist = info.diameterbounds;
        
        // compute inner distance for this point
        aPt min_intersect_pt;
        // Implicit function that computes the distance from a point x to the nearest point p on an input vtkPolyData.
        float dist = find_min_distance_point_2_polydata(pt,ptN,implicitPolyDataDistance,&min_intersect_pt);
        
        
        ////////////////////////////////////////
        ////    Negative distance
        ////////////////////////////////////////
        if (dist < 0.0) {
            num_times_negative_distance++;
            printf("OUTSIDE_WARNING:dist=%f:%f %f %f\n",dist,p[0],p[1],p[2]);
        }
        
        ////////////////////////////////////////
        ////    Positive distance
        ////////////////////////////////////////
        else {
            
            this_min_dist = dist;
            v.push_back(this_min_dist);
            
            if ((this_min_dist > min_mm) && (this_min_dist < max_mm)) {
                num_times_in_user_range++;
//                printf("INSIDE:%f %f %f\n",p[0],p[1],p[2]);
            }
            
#if EXTRA_DEBUG
            printf("dist=%f\n",this_min_dist);
#endif
            min_dist_clr[idx] = this_min_dist;
            if (this_min_dist < min_dist)
                min_dist = this_min_dist;
        }
    }
    
    min_dist = (min_dist == info.diameterbounds) ? 0.0 : min_dist;
    printf("CLEARANCE:MinDist=%f\n",min_dist);
    
    // write avg and stddev in csv format
    double sum = std::accumulate(v.begin(), v.end(), 0.0);
    double mean = sum / v.size();
    
    std::vector<double> diff(v.size());
    std::transform(v.begin(), v.end(), diff.begin(),
                   std::bind2nd(std::minus<double>(), mean));
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / v.size());
    
    printf("CLEARANCE:Times_negative_distance:%d\n",num_times_negative_distance);
    printf("CLEARANCE:Times_in_range[%f,%f]:%d\n",min_mm,max_mm,num_times_in_user_range);
    printf("CLEARANCE:# avg,stddev\n");
    printf("CLEARANCE:%f,%f\n",mean,stdev);
    
    
    
    
    if (cl_info) {
        (*cl_info).times_in_negative_distance = num_times_negative_distance;
        (*cl_info).times_in_range = num_times_in_user_range;
        (*cl_info).mean_mm = mean;
        (*cl_info).stddev_mm = stdev;
        (*cl_info).min_dist = min_dist;
    }
    return true;
}

