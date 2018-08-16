
cmake_minimum_required(VERSION 2.8)

PROJECT(make_stl_solid)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(make_stl_solid MACOSX_BUNDLE make_stl_solid)

if(VTK_LIBRARIES)
  target_link_libraries(make_stl_solid ${VTK_LIBRARIES})
else()
  target_link_libraries(make_stl_solid vtkHybrid vtkWidgets)
endif()
