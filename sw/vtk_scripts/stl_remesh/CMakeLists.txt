
cmake_minimum_required(VERSION 2.8)

PROJECT(stl_remesh)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(stl_remesh MACOSX_BUNDLE stl_remesh)

if(VTK_LIBRARIES)
  target_link_libraries(stl_remesh ${VTK_LIBRARIES})
else()
  target_link_libraries(stl_remesh vtkHybrid vtkWidgets)
endif()
