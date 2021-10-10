cmake_minimum_required(VERSION 3.5)

project(state_machine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(state_machine main.cpp util/arrays.h util/static_string.h)
