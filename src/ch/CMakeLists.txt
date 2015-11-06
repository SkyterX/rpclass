cmake_minimum_required (VERSION 3.0)
project (ch)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/thirdparty")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/tools")
#
# Project Output Paths
#
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

#
# Libraries 
#
#
# Project Search Paths
#
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
								 ${graph_INCLUDE_DIRS})
include_directories(${${PROJECT_NAME}_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(test)

