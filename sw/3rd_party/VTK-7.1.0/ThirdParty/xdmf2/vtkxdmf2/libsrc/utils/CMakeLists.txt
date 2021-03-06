INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/Utilities/exodusii/include
) 

SET(XdmfUtilSource
    XdmfDiff
    XdmfExodusReader
    XdmfExodusWriter
    XdmfFortran
    XdmfPartitioner
)

IF(XDMF_BUILD_MPI)
  SET(XDMF_UTILS_PYTHON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/XdmfUtils.py)
ELSE()
  SET(XDMF_UTILS_PYTHON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/XdmfUtilsNoMpi.py)
ENDIF()

IF(XDMF_REGENERATE_WRAPPERS)
  IF(SWIG_EXECUTABLE)
    FOREACH(LANGUAGE Python)
      STRING(TOLOWER ${LANGUAGE} lang)
      SET(XDMF_SWIG_INCLUDES ${XDMF_SWIG_INCLUDES} -I${CMAKE_CURRENT_SOURCE_DIR})
      IF(XDMF_BUILD_MPI)
        SET(swigtarget ${CMAKE_CURRENT_BINARY_DIR}/XdmfUtils${LANGUAGE}.cxx)
        MESSAGE(${swigtarget})
        ADD_CUSTOM_COMMAND(
          OUTPUT ${swigtarget}
          DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/XdmfUtils.i
          COMMAND ${SWIG_EXECUTABLE}
          ARGS -v -c++ -make_default -includeall "-${lang}" ${swig_extra_args} ${XDMF_SWIG_INCLUDES} -o "${swigtarget}" XdmfUtils.i)
      ELSE()
        SET(swigtarget ${Xdmf_BINARY_DIR}/libsrc/utils/XdmfUtils${LANGUAGE}NoMpi.cxx)
        ADD_CUSTOM_COMMAND(
         OUTPUT ${swigtarget}
         DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/XdmfUtils.i
         COMMAND ${SWIG_EXECUTABLE}
         ARGS -v -c++ -DXDMF_NO_MPI -make_default -includeall "-${lang}" ${swig_extra_args} ${XDMF_SWIG_INCLUDES} -o "${swigtarget}" XdmfUtils.i)
      ENDIF()
    ENDFOREACH()
    # ADD_CUSTOM_TARGET(RerunSwig DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/XdmfUtilsPython.cxx)
  ENDIF()
  SET(XDMF_UTILS_PYTHON_FILE ${CMAKE_CURRENT_BINARY_DIR}/XdmfUtils.py)
ENDIF()

IF(XDMF_WRAP_PYTHON)
  IF(XDMF_BUILD_MPI)
    ADD_LIBRARY(_XdmfUtils MODULE XdmfUtilsPython.cxx)
  ELSE()
    ADD_LIBRARY(_XdmfUtils MODULE XdmfUtilsPythonNoMpi.cxx)
  ENDIF()
  TARGET_LINK_LIBRARIES(_XdmfUtils XdmfUtils)
  IF(WIN32 OR APPLE)
    TARGET_LINK_LIBRARIES(_XdmfUtils ${PYTHON_LIBRARY})
  ENDIF()
  SET_TARGET_PROPERTIES(_XdmfUtils PROPERTIES PREFIX "")
  ADD_DEPENDENCIES(_XdmfUtils XdmfUtils)

  CONFIGURE_FILE(${XDMF_UTILS_PYTHON_FILE} XdmfUtils.py COPYONLY)

  INSTALL(FILES ${XDMF_UTILS_PYTHON_FILE}
      DESTINATION ${XDMF_WRAP_PYTHON_INSTALL_DIR}
      COMPONENT Runtime
      RENAME XdmfUtils.py # Always named the same regardless of build with or without mpi
  )
ENDIF()

ADD_LIBRARY(XdmfUtils ${LIBTYPE} ${XdmfUtilSource})
TARGET_LINK_LIBRARIES(XdmfUtils Xdmf ${XDMF_EXODUSII_LIBRARIES}
${XDMF_NETCDF_LIBRARIES} metis)

ADD_EXECUTABLE(XdmfDiff XdmfDiff)
ADD_EXECUTABLE(XdmfExodusConverter XdmfExodusConverter)
ADD_EXECUTABLE(XdmfPartitioner XdmfPartitioner)
SET_TARGET_PROPERTIES(XdmfDiff XdmfPartitioner PROPERTIES COMPILE_FLAGS -DBUILD_EXE)
TARGET_LINK_LIBRARIES(XdmfDiff XdmfUtils)
TARGET_LINK_LIBRARIES(XdmfExodusConverter XdmfUtils)
TARGET_LINK_LIBRARIES(XdmfPartitioner XdmfUtils)

IF(NOT XDMF_INSTALL_NO_DEVELOPMENT)
  FILE(GLOB devFiles RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
  INSTALL(
      FILES ${devFiles}
      DESTINATION ${XDMF_INSTALL_INCLUDE_DIR_CM24}/utils)
ENDIF()

IF(NOT XDMF_INSTALL_NO_LIBRARIES)
  INSTALL(TARGETS XdmfExodusConverter XdmfDiff XdmfPartitioner XdmfUtils
    RUNTIME DESTINATION ${XDMF_INSTALL_BIN_DIR_CM24} COMPONENT Runtime
    LIBRARY DESTINATION ${XDMF_INSTALL_LIB_DIR_CM24} COMPONENT Runtime
    ARCHIVE DESTINATION ${XDMF_INSTALL_LIB_DIR_CM24} COMPONENT Development)
  INSTALL(
    FILES XdmfSTLConverter.txx
    DESTINATION ${XDMF_INSTALL_INCLUDE_DIR_CM24})
  IF(XDMF_WRAP_PYTHON)
    INSTALL(
        TARGETS _XdmfUtils
        DESTINATION
        ${XDMF_WRAP_PYTHON_INSTALL_DIR})
  ENDIF()
ENDIF()
