cmake_minimum_required (VERSION 3.0)
project (rpclass)

# Lang standart

set(CMAKE_CXX_STANDARD_REQUIRED  ON)

add_definitions(-std=c++14)

SET(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_ITERATOR_DEBUG_LEVEL=1")

#
# Project Search Paths
#
include_directories("${PROJECT_SOURCE_DIR}/include")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/tools")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/thirdparty")
#
# Project Output Paths
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
#
#
# Libraries 
#
link_directories("${PROJECT_SOURCE_DIR}/lib")


#
# Theirdparty dependencies
#
# Google Test
#
set(gtest_force_shared_crt ON)
add_subdirectory(thirdparty/googletest-release-1.7.0)
#
#BOOST
#
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/thirdparty/boost")

#
# Subprojects
#
add_subdirectory(src/util)
add_subdirectory(src/graph)
add_subdirectory(src/arc-flags)
add_subdirectory(src/ch)
#add_subdirectory(test)