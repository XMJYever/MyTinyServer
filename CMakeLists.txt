cmake_minimum_required(VERSION 3.17)
project(MyTinyServer)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(. DIRSRCS)

include_directories(./processpool)
SET(PROCESS_SRC processpool/processpool_server.cpp)
add_executable(MyTinyServer ${PROCESS_SRC})