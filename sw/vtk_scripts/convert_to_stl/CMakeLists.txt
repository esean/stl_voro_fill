
cmake_minimum_required(VERSION 2.8)

PROJECT(convert_to_stl)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(convert_to_stl MACOSX_BUNDLE convert_to_stl)

if(VTK_LIBRARIES)
  target_link_libraries(convert_to_stl ${VTK_LIBRARIES})
else()
  target_link_libraries(convert_to_stl vtkHybrid vtkWidgets)
endif()
