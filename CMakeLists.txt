cmake_minimum_required(VERSION 3.16)

project(main)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include(ExternalProject)
#https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-4.1.2.tar.gz
ExternalProject_Add(
        petsc_external

        URL https://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-3.17.3.tar.gz
        URL_MD5 c27cd0b1f42ad408babda354de6bbb95

        BUILD_IN_SOURCE 1
        SOURCE_DIR ${CMAKE_BINARY_DIR}/toyapp/external/petsc/

        CONFIGURE_COMMAND
        ${CMAKE_BINARY_DIR}/toyapp/external/petsc/configure
        PETSC_DIR=${CMAKE_BINARY_DIR}/toyapp/external/petsc
        PETSC_ARCH=arch-linux-c-opt all
        --with-cc=mpicc --with-cxx=mpicxx --with-fc=0 --with-pic=1 --download-f2cblaslapack --download-metis --download-mpich --with-cxx-dialect=C++11 MAKEFLAGS=$MAKEFLAGS COPTFLAGS=-O3 -march=native -mtune=native CXXOPTFLAGS=-O3 -march=native -mtune=native --with-debugging=0

        BUILD_COMMAND
        make -j
        PETSC_DIR=${CMAKE_BINARY_DIR}/toyapp/external/petsc/
        PETSC_ARCH=arch-linux-c-opt all

        INSTALL_COMMAND ""
)

find_package(Boost COMPONENTS filesystem atomic REQUIRED)

add_library(toy SHARED ExampleClass.cpp ExampleClass.h)

add_executable(main main.cpp)
add_executable(Hello2 Hello2.cpp)

ExternalProject_Get_Property(petsc_external SOURCE_DIR)
set(PETSC_INCLUDES ${SOURCE_DIR}include)
set(PETSC_INCLUDES_CONFIG ${SOURCE_DIR}arch-linux-c-opt/include)
set(PETSC_LIBRARIES ${SOURCE_DIR}arch-linux-c-opt/lib/libpetsc.so.3.17)
MESSAGE("BANDERA 1 " ${PETSC_INCLUDES})
target_include_directories(main PRIVATE "${PETSC_INCLUDES}")
target_include_directories(main PRIVATE "${PETSC_INCLUDES_CONFIG}")

target_include_directories(toy PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(main PRIVATE ${Boost_LIBRARIES})
target_link_libraries(main ${PETSC_LIBRARIES})

install(TARGETS toy DESTINATION bin)
include(InstallRequiredSystemLibraries)

enable_testing()
add_test(NAME main COMMAND main)
add_test(NAME hello2 COMMAND Hello2)

set(CPACK_PACKAGE_FILE_NAME "toyapp")
include(CPack)


