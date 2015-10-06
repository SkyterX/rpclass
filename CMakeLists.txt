cmake_minimum_required (VERSION 3.0)
project (rpclass)

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

list(APPEND CMAKE_CXX_FLAGS " -std=c++11")

add_subdirectory(thirdparty/googletest-release-1.7.0)

add_subdirectory(src/graph)
#add_subdirectory(test)