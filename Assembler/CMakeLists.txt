cmake_minimum_required(VERSION 3.12)
project(SystemSoftware2)

set(CMAKE_CXX_STANDARD 14)

add_executable(SystemSoftware2 main.cpp Assembler.cpp Assembler.h Operator.h Errors.h MParser.h MParser.cpp MClient.cpp MClient.h StrToDate.h Listing.cpp Listing.h)