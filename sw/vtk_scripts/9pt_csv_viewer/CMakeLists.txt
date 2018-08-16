
cmake_minimum_required(VERSION 2.8)

PROJECT(9pt_csv_viewer)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(9pt_csv_viewer MACOSX_BUNDLE 9pt_csv_viewer)

if(VTK_LIBRARIES)
  target_link_libraries(9pt_csv_viewer ${VTK_LIBRARIES})
else()
  target_link_libraries(9pt_csv_viewer vtkHybrid vtkWidgets)
endif()
