cmake_minimum_required(VERSION 3.5)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
project(stgmgr)

set(CMAKE_CXX_STANDARD 11)

add_executable(stgmgr src/main.cpp src/Page.cpp src/Page.h src/constants.h src/Disc.cpp src/Disc.h)