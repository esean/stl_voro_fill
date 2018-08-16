
cmake_minimum_required(VERSION 2.8)

PROJECT(ViewSTL)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(ViewSTL MACOSX_BUNDLE ViewSTL)

if(VTK_LIBRARIES)
  target_link_libraries(ViewSTL ${VTK_LIBRARIES})
else()
  target_link_libraries(ViewSTL vtkHybrid vtkWidgets)
endif()
