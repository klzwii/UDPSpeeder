cmake_minimum_required(VERSION 3.13)
project(UDPSpeeder)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-pthread -l netfilter_queue")

include_directories(${PROJECT_SOURCE_DIR}/lib/crc32/include)
link_libraries(${PROJECT_SOURCE_DIR}/lib/crc32/libcrc32c.a)

add_executable(UDPSpeeder UDPSpeeder.cpp RSHelper.cpp RSHelper.h SimpleWorkQueue/SimpleWorker.h SimpleWorkQueue/SimpleTask.h SimpleWorkQueue/SimpleWorkQueue.cpp SimpleWorkQueue/SimpleWorkQueue.h PackageProcessor/PackageProcessWorker.cpp PackageProcessor/PackageProcessWorker.h PackageProcessor/PackageProcessTask.cpp PackageProcessor/PackageProcessTask.h NetFilterHook.h)