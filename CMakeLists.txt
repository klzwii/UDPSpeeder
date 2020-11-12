# cmake_minimum_required(VERSION <specify CMake version here>)
project(UDPSpeeder)

set(CMAKE_CXX_STANDARD 14)

add_executable(UDPSpeeder UDPSpeeder.cpp RSHelper.cpp RSHelper.h)