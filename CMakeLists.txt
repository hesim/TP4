cmake_minimum_required(VERSION 3.10)
project(TP4)

set(CMAKE_CXX_STANDARD 11)

include_directories(Divers)
include_directories(TP4/lib)
include_directories(TP4/PACC)

add_executable(TP4
        Divers/Chrono.hpp
        TP4/lib/Chrono.hpp
        TP4/lib/lodepng.cpp
        TP4/lib/lodepng.h
        TP4/PACC/Assert.hpp
        TP4/PACC/config.hpp
        TP4/PACC/Tokenizer.cpp
        TP4/PACC/Tokenizer.hpp
        TP4/tp4.cpp)
