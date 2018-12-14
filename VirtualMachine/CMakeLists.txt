cmake_minimum_required(VERSION 3.10)
project(SystemSoftware1)

set(CMAKE_CXX_STANDARD 17)

add_executable(SystemSoftware1 main.cpp Processor.cpp Processor.h Commands.h Type.h OpCodes.h Loaders.h Debugger.cpp Debugger.h)