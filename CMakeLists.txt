cmake_minimum_required(VERSION 3.17)
project(MyTinyServer)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(. DIRSRCS)

include_directories(./processpool)
set(PROCESS_SRC processpool/processpool_server.cpp)
add_executable(MyTinyServer processpool/processpool_server.cpp processpool/processpool.h processpool/processpool_server.h http_conn.cpp http_conn.h)