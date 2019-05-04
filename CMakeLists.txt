cmake_minimum_required(VERSION 3.0)

aux_source_directory(. DIR_SRCS)
add_executable(mpeg-decoder ${DIR_SRCS})

find_package(SDL2)
target_link_libraries(mpeg-decoder SDL2)
