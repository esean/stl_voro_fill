
cmake_minimum_required(VERSION 2.8)

PROJECT(TubesFromSplines)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(TubesFromSplines MACOSX_BUNDLE TubesFromSplines)

if(VTK_LIBRARIES)
  target_link_libraries(TubesFromSplines ${VTK_LIBRARIES})
else()
  target_link_libraries(TubesFromSplines vtkHybrid vtkWidgets)
endif()
