cmake_minimum_required(VERSION 3.16)

project(main)

find_package(Boost COMPONENTS filesystem atomic REQUIRED)

add_executable(main main.cpp)
add_executable(Hello2 Hello2.cpp)

target_link_libraries(main PRIVATE ${Boost_LIBRARIES})

install(TARGETS main)

enable_testing()
add_test(NAME main COMMAND main)
add_test(NAME hello2 COMMAND Hello2)

