cmake_minimum_required (VERSION 3.0)
project (graph)

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
# BOOST
#
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/thirdparty/boost")

find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
#
# Project Search Paths
#
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
								 ${Boost_INCLUDE_DIRS}
	CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
include_directories(${${PROJECT_NAME}_INCLUDE_DIRS})
#
# Project Lib Paths
#
link_directories(${Boost_LIBRARY_DIRS})
link_directories("${PROJECT_SOURCE_DIR}/lib")

add_subdirectory(src)
add_subdirectory(test)

