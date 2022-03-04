cmake_minimum_required(VERSION 3.5)

aux_source_directory(. SRC_LIST)
add_executable(webserver ${SRC_LIST})
include_directories(src)
link_directories(src)
target_link_libraries(webserver pthread)