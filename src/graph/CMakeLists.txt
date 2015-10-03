cmake_minimum_required (VERSION 3.0)
project (graph)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#
# Project Search Paths
#
include_directories("${PROJECT_SOURCE_DIR}/include")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/thirdparty")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/tools")
#
# Project Output Paths
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

#
# Libraries 
#
# BOOST 
#set(BOOST_MIN_VERSION "1.49.0")
#list(APPEND BOOST_COMPONENTS "program_options")
#list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/thirdparty/boost")
#find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
#include_directories(${Boost_INCLUDE_DIRS})
#link_directories(${Boost_LIBRARY_DIRS})

link_directories("${PROJECT_SOURCE_DIR}/lib")

list(APPEND CMAKE_CXX_FLAGS " -std=c++11")

add_subdirectory(src)
add_subdirectory(test)

