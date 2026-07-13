cmake_minimum_required(VERSION 3.28)
project(app CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(app main.cpp)

target_sources(app
  PUBLIC
    FILE_SET CXX_MODULES FILES
      src/math.cppm
      src/types.cppm
)
