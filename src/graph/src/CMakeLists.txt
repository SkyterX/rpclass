#
# Project Sources
#
set(PROJECT_HEADERS_DIR "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}")
file(GLOB_RECURSE PROJECT_HEADERS ${PROJECT_HEADERS_DIR}/*.h ${PROJECT_HEADERS_DIR}/*.hpp)
file(GLOB_RECURSE PROJECT_SRCS *.cpp *.h *.hpp)

foreach(f ${PROJECT_HEADERS})
    # Get the path of the file relative to ${DIRECTORY},
    # then alter it (not compulsory)
    file(RELATIVE_PATH SRCGR "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}" ${f})
    set(SRCGR "Header Files/${SRCGR}")

    # Extract the folder, ie remove the filename part
    string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})

    # Source_group expects \\ (double antislash), not / (slash)
    string(REPLACE / \\ SRCGR ${SRCGR})
    source_group("${SRCGR}" FILES ${f})
endforeach()

add_executable(${PROJECT_NAME} ${PROJECT_HEADERS} ${PROJECT_SRCS} )

#
# Set compiler flags
#

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
# message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

target_link_libraries(${PROJECT_NAME})

#
# Add Install Targets
#

install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib)

if(EXISTS "${PROJECT_HEADERS_DIR}" AND IS_DIRECTORY "${PROJECT_HEADERS_DIR}")
	install(DIRECTORY ${PROJECT_HEADERS_DIR} DESTINATION "include")
endif(EXISTS "${PROJECT_HEADERS_DIR}" AND IS_DIRECTORY "${PROJECT_HEADERS_DIR}")
