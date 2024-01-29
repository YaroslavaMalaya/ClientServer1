cmake_minimum_required(VERSION 3.26)
project(lesson1)

set(CMAKE_CXX_STANDARD 14)

include_directories(client)

add_executable(client1 client.cpp)

add_executable(client2 client.cpp)

add_executable(server server.cpp)