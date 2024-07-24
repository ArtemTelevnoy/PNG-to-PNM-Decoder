cmake_minimum_required(VERSION 3.24)
project(lab2 C ASM)

set(CMAKE_C_STANDARD 11)

aux_source_directory(./isa-l/igzip SRC)
aux_source_directory(./isa-l/crc SRC)
aux_source_directory(./isa-l/include SRC)
add_library(igzip ${SRC})
target_include_directories(igzip PUBLIC ./isa-l ./isa-l/include ./isa-l/igzip )

add_subdirectory(./libdeflate)

add_executable(lab2 main.c my_header.h)

target_link_libraries(lab2 igzip)
target_link_libraries(lab2 libdeflate_static z)

target_compile_definitions(lab2 PRIVATE
        ZLIB
        #LIBDEFLATE
        #ISAL
        )