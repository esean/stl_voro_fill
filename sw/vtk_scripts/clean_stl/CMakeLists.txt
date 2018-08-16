
cmake_minimum_required(VERSION 2.8)

PROJECT(clean_stl)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(clean_stl MACOSX_BUNDLE clean_stl)

if(VTK_LIBRARIES)
  target_link_libraries(clean_stl ${VTK_LIBRARIES})
else()
  target_link_libraries(clean_stl vtkHybrid vtkWidgets)
endif()
