cmake_minimum_required(VERSION 3.19)
project(rnnoise_toys C)

set(CMAKE_C_STANDARD 11)

add_executable(rnnoise_toys
        examples/rnnoise_demo.c)
