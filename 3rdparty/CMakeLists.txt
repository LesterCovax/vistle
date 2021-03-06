#project(vistle_3rdparty)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

if (MSVC)
   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4267 /wd4800 /wd4244")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4996 /wd4267 /wd4800 /wd4244")
endif()

if (NOT VISTLE_GUI_ONLY)
    include(boost-mpi.cmake)
endif()

if (CRAY)
   set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
endif()

if (NOT BUILD_SHARED_LIBS)
   set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
endif()

set(BUILD_SHARED_LIBS ${VISTLE_BUILD_SHARED} CACHE INTERNAL "")
set(BUILD_TESTING OFF CACHE INTERNAL "")
set(BUILD_UTILITIES OFF CACHE INTERNAL "")
set(ZFP_LIBRARY_PREFIX "vistle_" CACHE STRING "Prefix to prepend to the output library name")
mark_as_advanced(ZFP_LIBRARY_PREFIX)
add_subdirectory(zfp)

set(BUILD_SHARED_LIBS ${VISTLE_BUILD_SHARED} CACHE INTERNAL "")
set(ICET_BUILD_TESTING OFF CACHE INTERNAL "")
set(ICET_USE_OPENGL OFF CACHE INTERNAL "")
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Output directory for building all libraries.")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib CACHE PATH "Output directory for building all binaries.")
SET(ICET_LIBRARY_DIR ${PROJECT_BINARY_DIR}/lib CACHE PATH "Output directory for building all libraries.")
SET(ICET_EXECUTABLE_DIR ${PROJECT_BINARY_DIR}/bin CACHE PATH "Output directory for building all executable.")
set(ICET_BINARY_DIR ${PROJECT_BINARY_DIR} CACHE PATH "Value overriden by Vistle")
set(ICET_INSTALL_LIB_DIR ${INSTALL_LIB_DIR} CACHE INTERNAL "")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-variadic-macros -Wno-pedantic)
endif()

if (NOT VISTLE_GUI_ONLY)
    add_subdirectory(IceT)

    set(PythonLibsNew_FIND_REQUIRED FALSE)
    set(PythonLibsNew_FOUND TRUE)
    add_subdirectory(pybind11)
endif()
