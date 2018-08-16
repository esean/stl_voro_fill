
cmake_minimum_required(VERSION 2.8)

PROJECT(stl_rotate)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(stl_rotate MACOSX_BUNDLE stl_rotate)

if(VTK_LIBRARIES)
  target_link_libraries(stl_rotate ${VTK_LIBRARIES})
else()
  target_link_libraries(stl_rotate vtkHybrid vtkWidgets)
endif()
