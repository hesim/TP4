cmake_minimum_required(VERSION 3.9)
project(tp2)

#set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_COMPILER /usr/local/bin/g++-7)
set(CMAKE_C_COMPILER /usr/local/bin/gcc-7)
# enable openmp using -fopenmp
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")

add_executable(tp4
        PACC/Assert.hpp
        PACC/config.hpp
        PACC/Tokenizer.cpp
        PACC/Tokenizer.hpp
        lib/Chrono.hpp
        exemple.png
        lib/lodepng.cpp
        lib/lodepng.h
        makefile
        noyau_flou
        noyau_identite
        tp4.cpp
        )
