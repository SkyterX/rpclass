#
# Test Sources
#
file(GLOB_RECURSE TEST_SRCS *.cpp *.h *.hpp)

#
# Google Test
#
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
#
# Build Test Executables
#
add_executable(${PROJECT_NAME}-test ${TEST_SRCS})
target_link_libraries(${PROJECT_NAME}-test gtest gtest_main)
#
# Set compiler flags
#
set_property(TARGET ${PROJECT_NAME}-test PROPERTY CXX_STANDARD 11) 
#
# Add Install Targets
#
install (TARGETS ${PROJECT_NAME}-test RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
