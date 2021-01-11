cmake_minimum_required(VERSION 3.17)
project(MyTinyServer)

set(CMAKE_CXX_STANDARD 14)

add_executable(MyTinyServer main.cpp locker.h processpool.h processpool.cpp)