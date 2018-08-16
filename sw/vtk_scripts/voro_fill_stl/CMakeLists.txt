
cmake_minimum_required(VERSION 2.8)

PROJECT(voro_fill_stl)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(VORO_INSTALL_LIB ${PROJECT_SOURCE_DIR}/../../3rd_party/voro++-0.4.6/src)
find_library(LIB_LOCATION voro++ ${VORO_INSTALL_LIB})
if(NOT LIB_LOCATION)
  message(FATAL_ERROR "voro++ library not found")
endif()
include_directories(${VORO_INSTALL_LIB})
# dbg
#get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
#message("inc_dirs = ${inc_dirs}")

add_executable(voro_fill_stl MACOSX_BUNDLE voro_fill_stl)

if(VTK_LIBRARIES)
  target_link_libraries(voro_fill_stl ${VTK_LIBRARIES} ${LIB_LOCATION})
else()
  target_link_libraries(voro_fill_stl vtkHybrid vtkWidgets ${LIB_LOCATION})
endif()
