
cmake_minimum_required(VERSION 2.8)

PROJECT(stl_info)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(stl_info MACOSX_BUNDLE stl_info)

if(VTK_LIBRARIES)
  target_link_libraries(stl_info ${VTK_LIBRARIES})
else()
  target_link_libraries(stl_info vtkHybrid vtkWidgets)
endif()
