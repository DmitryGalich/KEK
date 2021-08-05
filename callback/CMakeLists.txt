cmake_minimum_required(VERSION 3.10)

set(CALLBACK callback)

add_library(${CALLBACK}
    ${CALLBACK}.h
    ${CALLBACK}.cpp
)
add_library(Libs::Callback ALIAS ${CALLBACK})

target_include_directories (${CALLBACK}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

