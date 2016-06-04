cmake_minimum_required(VERSION 2.6)
project(kuonline)

include_directories("${PROJECT_SOURCE_DIR}/../seasocks/src/main/c/")
include_directories("${PROJECT_SOURCE_DIR}")

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -pthread")

set(SOURCES
	main.cpp
	)

# Define sources and executable
set(EXECUTABLE_NAME "kuserver")
add_executable(${EXECUTABLE_NAME} ${SOURCES})

#link_directories("${PROJECT_SOURCE_DIR}/lib")

target_link_libraries(${EXECUTABLE_NAME} "${PROJECT_SOURCE_DIR}/lib/libseasocks.so")

#add_custom_command(TARGET kuserver POST_BUILD   COMMAND ${CMAKE_COMMAND} -E copy_directory   ${PROJECT_SOURCE_DIR}/web ${PROJECT_BINARY_DIR}/web)
