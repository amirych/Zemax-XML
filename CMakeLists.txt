project(Zemax-XML)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

