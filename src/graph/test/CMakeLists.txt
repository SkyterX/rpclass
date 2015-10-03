#
# Test Sources
#
file(GLOB_RECURSE TEST_SRCS *.cpp *.h *.hpp)

#
# Google Test
#
enable_testing()
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/thirdparty/gtest")
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
#
# Build Test Executables
#
add_executable(${PROJECT_NAME}-test ${TEST_SRCS})
target_link_libraries(${PROJECT_NAME}-test ${GTEST_BOTH_LIBRARIES})
#
# Set compiler flags
#
set_property(TARGET ${PROJECT_NAME}-test PROPERTY CXX_STANDARD 11) 
#
# Add Install Targets
#
install (TARGETS ${PROJECT_NAME}-test RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
