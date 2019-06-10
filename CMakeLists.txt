cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)

aux_source_directory(. DIR_SRCS)
add_executable(mpeg-decoder main.cpp decoder.cpp play_video.cpp bit_reader.cpp util.cpp image_queue.cpp)

find_package(Threads)

target_link_libraries(mpeg-decoder pthread -lsfml-graphics -lsfml-window -lsfml-system)
