#
# Test headers
#
file(GLOB_RECURSE ${PROJECT_NAME}_TEST_HEADERS include/*.h include/*.hpp)

#
# Other Test Sources
#
file(GLOB_RECURSE ${PROJECT_NAME}_OTHER_TEST_SRCS ./*.cpp ./*.h ./*.hpp)

#
# Build Unit Test Executables
#
add_executable(${PROJECT_NAME}-other ${${PROJECT_NAME}_OTHER_TEST_SRCS} ${${PROJECT_NAME}_TEST_HEADERS})
target_link_libraries(${PROJECT_NAME}-other gtest gtest_main)
#
# Set compiler flags
#

#
# Add Install Targets
#
install (TARGETS ${PROJECT_NAME}-other RUNTIME DESTINATION bin LIBRARY DESTINATION lib)