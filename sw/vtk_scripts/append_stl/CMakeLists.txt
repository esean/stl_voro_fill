
cmake_minimum_required(VERSION 2.8)

PROJECT(append_stl)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(append_stl MACOSX_BUNDLE append_stl)

if(VTK_LIBRARIES)
  target_link_libraries(append_stl ${VTK_LIBRARIES})
else()
  target_link_libraries(append_stl vtkHybrid vtkWidgets)
endif()
