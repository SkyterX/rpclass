#
# Google Test
#
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
include_directories(include)


#
# Test headers
#
file(GLOB_RECURSE ${PROJECT_NAME}_TEST_HEADERS include/*.h include/*.hpp)

#
# Unit Test Sources
#
file(GLOB_RECURSE ${PROJECT_NAME}_UNIT_TEST_SRCS unit/*.cpp unit/*.h unit/*.hpp)

#
# Build Unit Test Executables
#
add_executable(${PROJECT_NAME}-test ${${PROJECT_NAME}_UNIT_TEST_SRCS} ${${PROJECT_NAME}_TEST_HEADERS})
target_link_libraries(${PROJECT_NAME}-test gtest gtest_main)
#
# Set compiler flags
#

#
# Performance Test Sources
#
file(GLOB_RECURSE ${PROJECT_NAME}_PERFORMANCE_TEST_SRCS performance/*.cpp performance/*.h performance/*.hpp)
#
# Build Performance Test Executables
#
add_executable(${PROJECT_NAME}-performance-test ${${PROJECT_NAME}_PERFORMANCE_TEST_SRCS}  ${${PROJECT_NAME}_TEST_HEADERS})
target_link_libraries(${PROJECT_NAME}-performance-test gtest gtest_main)
#
# Set compiler flags
#
#
# Add Install Targets
#
install (TARGETS ${PROJECT_NAME}-test RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
install (TARGETS ${PROJECT_NAME}-performance-test RUNTIME DESTINATION bin LIBRARY DESTINATION lib)

include_directories(other)
