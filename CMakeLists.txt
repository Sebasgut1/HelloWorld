cmake_minimum_required(VERSION 3.16)

project(main)

find_package(MPI REQUIRED)

include_directories(${MPI_INCLUDE_PATH})
target_link_libraries(main ${MPI_LIBRARIES})

add_executable(main main.cpp)
add_executable(Hello2 Hello2.cpp)

if(MPI_COMPILE_FLAGS)
    set_target_properties(main PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
    set_target_properties(main PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

install(TARGETS main)

enable_testing()
add_test(NAME main COMMAND main)
add_test(NAME hello2 COMMAND Hello2)

