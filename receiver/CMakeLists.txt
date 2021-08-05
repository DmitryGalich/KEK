cmake_minimum_required(VERSION 3.10)

set(RECEIVER receiver)

add_library(${RECEIVER}
    ${RECEIVER}.h
    ${RECEIVER}.cpp
)
add_library(Libs::Receiver ALIAS ${RECEIVER})

# Test =======================================

set(RECEIVER_TEST receiver_test)

add_executable(${RECEIVER_TEST}
   ${RECEIVER_TEST}.cpp
)
target_link_libraries(${RECEIVER_TEST}
    PRIVATE
        Libs::Receiver
	Libs::Callback
        GTest::gtest_main
)
