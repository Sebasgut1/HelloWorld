cmake_minimum_required(VERSION 3.16)

project(main)

add_executable(main main.cpp)
add_executable(Hello2 Hello2.cpp)

install(TARGETS main)

enable_testing()
add_test(NAME main COMMAND main)
add_test(NAME hello2 COMMAND Hello2)
