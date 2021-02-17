# cmake_minimum_required(VERSION <specify CMake version here>)
project(UDPSpeeder)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS -lpthread)

add_executable(UDPSpeeder UDPSpeeder.cpp RSHelper.cpp RSHelper.h SimpleWorkQueue/SimpleWorker.h SimpleWorkQueue/SimpleTask.h SimpleWorkQueue/SimpleWorkQueue.cpp SimpleWorkQueue/SimpleWorkQueue.h PackageProcessor/PackageProcessWorker.cpp PackageProcessor/PackageProcessWorker.h PackageProcessor/PackageProcessTask.cpp PackageProcessor/PackageProcessTask.h)