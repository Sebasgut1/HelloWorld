cmake_minimum_required(VERSION 3.16)

project(main)

##     BITPIT      ############################################################
#fill the bitpit module list required by mimic.
set(BITPIT_QUERYPACKAGES "levelset" "surfunstructured" "volunstructured" "RBF" "CG" "volcartesian" "voloctree")
if(MODULE_ENABLED_PROPAGATORS)
    list (APPEND BITPIT_QUERYPACKAGES "discretization")
endif()
if(MODULE_ENABLED_LAPLACIANS)
    list (APPEND BITPIT_QUERYPACKAGES "LA")
endif()

find_package(BITPIT REQUIRED COMPONENTS ${BITPIT_QUERYPACKAGES})
include(${BITPIT_USE_FILE})


#see if BITPIT has MPI enabled
list (FIND BITPIT_DEFINITIONS "BITPIT_ENABLE_MPI=1" _index_DEF)
if (${_index_DEF} GREATER -1)
    if(NOT ENABLE_MPI)
        message( WARNING "FOUND BITPIT_MPI version, but mimic is required to be compiled SERIAL. Proceeding anyway")
    endif()
else()
    if(ENABLE_MPI)
        ## mimic required mpi but bitpit is Serial. That's not allowed.
        message( FATAL_ERROR "FOUND BITPIT_SERIAL version, but mimic is required to be compiled MPI. Cannot continue")
    endif()
endif()
unset(_index_DEF)

### OPENFOAM ###################################################################
list(FIND EXTERNAL_ACTIVE_DEPS "OPENFOAM" _OPENFOAM_index)
if (${_OPENFOAM_index} GREATER -1)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/openfoam/cmake")
    include(openFOAMSearch)
    openFOAMSearch(OFOAM_INCLUDES OFOAM_LIBS OFOAM_DEFINITIONS)

    list (APPEND MIMIC_EXTERNAL_DEPENDENCIES "OPENFOAM")
    list (APPEND MIMIC_EXTERNAL_LIBRARIES "${OFOAM_LIBS}")
    list (APPEND MIMIC_EXTERNAL_INCLUDE_DIRS "${OFOAM_INCLUDES}")

    addPublicDefinitions("${OFOAM_DEFINITIONS}")

else()
    cleanSingleCacheVariables("OPENFOAM")
endif ()

unset(_OPENFOAM_index)

find_package(Boost COMPONENTS filesystem atomic REQUIRED)

add_executable(main main.cpp)
add_executable(Hello2 Hello2.cpp)

target_link_libraries(main PRIVATE ${Boost_LIBRARIES})

install(TARGETS main)

enable_testing()
add_test(NAME main COMMAND main)
add_test(NAME hello2 COMMAND Hello2)

