cmake_minimum_required(VERSION 3.13)
project(UDPSpeeder)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-pthread -l netfilter_queue")

include_directories(${PROJECT_SOURCE_DIR}/lib/crc32/include)
link_libraries(${PROJECT_SOURCE_DIR}/lib/crc32/libcrc32c.a)
add_compile_definitions("server")
set(SOURCE_FILES UDPSpeeder.cpp Connection.h Connection.cpp RDT.cpp RDT.h GF.cpp GF.h RS.cpp RS.h NAT.h NAT.cpp IPPool.h LRULinkedList.cpp LRULinkedList.h LinkedNodeCallBack.h)
add_executable(UDPSpeeder ${SOURCE_FILES})