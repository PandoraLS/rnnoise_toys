cmake_minimum_required(VERSION 3.16)
project(rnnoise_toys C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
include_directories(src)

add_executable(rnnoise_toys
        examples/rnnoise_demo.c
        include/rnnoise.h
        src/_kiss_fft_guts.h
        src/arch.h
        src/celt_lpc.c
        src/celt_lpc.h
        src/common.h
        src/kiss_fft.c
        src/kiss_fft.h
        src/opus_types.h
        src/pitch.c
        src/pitch.h
        src/rnn.c
        src/rnn.h
        src/rnn_data.c
        src/rnn_data.h
        src/rnn_reader.c
        src/tansig_table.h
        src/denoise.c)

target_link_libraries(rnnoise_toys m)
