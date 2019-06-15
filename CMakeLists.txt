cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)


aux_source_directory(. DIR_SRCS)
add_executable(mpeg-decoder main.cpp decoder.cpp play_video.cpp bit_reader.cpp util.cpp image_queue.cpp qdbmp.cpp YCbCr.cpp)

target_compile_options(mpeg-decoder PRIVATE -Wall -Wextra -pedantic)
find_package(Threads)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

target_link_libraries(mpeg-decoder pthread -lsfml-graphics -lsfml-window -lsfml-system)
