cmake_minimum_required(VERSION 3.19)
project(rnnoise_toys C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(src)

add_executable(rnnoise_toys
        examples/rnnoise_demo.c
        include/rnnoise.h
        src/denoise.c
        src/rnn.c
        src/rnn.h
        src/rnn_data.h)
