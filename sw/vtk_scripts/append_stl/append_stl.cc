// Use VTK to combine 2 STL
// 6-9-17 - mess created
//

#include "../common/vtk_common.h"

#define VTK_CREATE(type, name)  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()


void usage(char* str)
{
    shared_print_version();
    printf("\nUSAGE: %s {ARGS} [STL #1] [STL #2] ... {STL #N}\n",str);
    cout << "\nCombines STLs together" << endl;
    cout << "OUTPUT: writes to [STL]-union.stl" << endl;
    cout << endl;
}

int main(int argc, char *argv[])
{
    int argc_cnt = 1;
    if (argc < 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    // open STL
    std::string inputFilename = argv[argc_cnt++];
    printf("# DEBUGCFG:%s:IN STL #1 = %s\n",argv[0],inputFilename.c_str());
    vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
    if (!read_stl(&reader,inputFilename,true))
        return -1;
    
    vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
    
    vtkSmartPointer<vtkPolyData> stl1 = vtkSmartPointer<vtkPolyData>::New();
    stl1->ShallowCopy(reader->GetOutput());
    appendFilter->AddInputData(stl1);
    
    
    
    while (argc_cnt < argc) {
        //printf("argc=%d argc_cnt=%d\n",argc,argc_cnt);
        
        std::string inputFilename2 = argv[argc_cnt++];
        printf("# DEBUGCFG:%s:IN STL #%d = %s\n",argv[0],argc_cnt-1,inputFilename2.c_str());
        vtkSmartPointer<vtkSTLReader> reader2 = vtkSmartPointer<vtkSTLReader>::New();
        if (!read_stl(&reader2,inputFilename2,true))
            return -1;
        
        vtkSmartPointer<vtkPolyData> stl2 = vtkSmartPointer<vtkPolyData>::New();
        stl2->ShallowCopy(reader2->GetOutput());
        appendFilter->AddInputData(stl2);
    }
    
    
    appendFilter->Update();
    
    // For debug: output STL
    if (!write_stl(inputFilename,appendFilter->GetOutput(),"union",true))
        return -1;
    
    return 0;
}

