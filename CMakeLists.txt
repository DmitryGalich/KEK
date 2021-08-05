cmake_minimum_required(VERSION 3.10)

project(ISS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -Werror)

find_package(GTest 1.10 REQUIRED)

add_subdirectory(callback)
add_subdirectory(receiver)

