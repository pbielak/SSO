cmake_minimum_required(VERSION 3.8)
project(LAB7)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        chat-client.c
        chat-server.c
        Makefile)

add_executable(LAB7 ${SOURCE_FILES})