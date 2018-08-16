
cmake_minimum_required(VERSION 2.8)

PROJECT(translate_stl)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(translate_stl MACOSX_BUNDLE translate_stl)

if(VTK_LIBRARIES)
  target_link_libraries(translate_stl ${VTK_LIBRARIES})
else()
  target_link_libraries(translate_stl vtkHybrid vtkWidgets)
endif()
