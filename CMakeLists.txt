cmake_minimum_required(VERSION 2.8)
project(ffmpegmuxer)

SET(CMAKE_CXX_FLAGS_DEBUG " $ENV{CXXFLAGS}   -std=c++11  -DDEBUG -g -O0 -ggdb3 -w ")
SET(CMAKE_CXX_FLAGS_RELEASE " $ENV{CXXFLAGS} -std=c++11  -DNDEBUG -g -O3 -w ")
SET(CMAKE_BUILD_TYPE "Debug")

include_directories(/data/App/ffmpeg/include)
link_directories(/data/App/ffmpeg/lib)

file(GLOB_RECURSE ffmpegmuxer_src src/*.cc src/*.cpp src/*.h)

set(MUXER_LIB
        x264
        avformat avcodec avutil swscale
        pthread rt dl z
        )

add_executable(${PROJECT_NAME} ${ffmpegmuxer_src} )
target_link_libraries(${PROJECT_NAME} ${MUXER_LIB})

