cmake_minimum_required(VERSION 3.16)

project(main)

#----------------------------------------------------------------------------
#
#  mimic
#
#  Optimad Engineering S.r.l. ("COMPANY") CONFIDENTIAL
#  Copyright (c) 2015-2022 Optimad Engineering S.r.l., All Rights Reserved.
#
#  --------------------------------------------------------------------------
#
#  NOTICE:  All information contained herein is, and remains the property
#  of COMPANY. The intellectual and technical concepts contained herein are
#  proprietary to COMPANY and may be covered by Italian and Foreign Patents,
#  patents in process, and are protected by trade secret or copyright law.
#  Dissemination of this information or reproduction of this material is
#  strictly forbidden unless prior written permission is obtained from
#  COMPANY. Access to the source code contained herein is hereby forbidden
#  to anyone except current COMPANY employees, managers or contractors who
#  have executed Confidentiality and Non-disclosure agreements explicitly
#  covering such access.
#
#  The copyright notice above does not evidence any actual or intended
#  publication or disclosure of this source code, which includes information
#  that is confidential and/or proprietary, and is a trade secret, of
#  COMPANY. ANY REPRODUCTION, MODIFICATION, DISTRIBUTION, PUBLIC PERFORMANCE,
#  OR PUBLIC DISPLAY OF OR THROUGH USE  OF THIS  SOURCE CODE  WITHOUT THE
#  EXPRESS WRITTEN CONSENT OF COMPANY IS STRICTLY PROHIBITED, AND IN
#  VIOLATION OF APPLICABLE LAWS AND INTERNATIONAL TREATIES. THE RECEIPT OR
#  POSSESSION OF THIS SOURCE CODE AND/OR RELATED INFORMATION DOES NOT CONVEY
#  OR IMPLY ANY RIGHTS TO REPRODUCE, DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR
#  TO MANUFACTURE, USE, OR SELL ANYTHING THAT IT  MAY DESCRIBE, IN WHOLE OR
#  IN PART.
#
#----------------------------------------------------------------------------
#------------------------------------------------------------------------------------#
# Variables visible to the user
#------------------------------------------------------------------------------------#
set(ENABLE_MPI 0 CACHE BOOL "If set, the program is compiled with MPI support")
set(VERBOSE_MAKE 0 CACHE BOOL "Set appropriate compiler and cmake flags to enable verbose output from compilation")
set(BUILD_SHARED_LIBS 1 CACHE BOOL "Build Shared Libraries")
set(BUILD_MIMIC_API_MODE 0 CACHE BOOL "Compile mimic as pure API for redistribution purposes")

# Create the build type variable
if(NOT CMAKE_BUILD_TYPE)
    # Set default build type to Debug
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
            "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel GNUProfiling ScalascaProfiling."
            FORCE)

    # Set the possible values of build type for the GUI
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
            "MinSizeRel" "RelWithDebInfo" "GNUProfiling" "ScalascaProfiling")
endif()

# Create the LTO_STRATEGY variable to set up if possible a Link Time Optimization for shared libs
if(${CMAKE_VERSION} VERSION_EQUAL "3.9.0" OR ${CMAKE_VERSION} VERSION_GREATER "3.9.0")
    set(LTO_STRATEGY "Auto" CACHE STRING "Choose the Link Time Optimization (LTO) strategy, options are: Auto (i.e., optimization is enabled only in release build) Enabled Disabled.")
    set_property(CACHE LTO_STRATEGY PROPERTY STRINGS "Auto" "Enabled" "Disabled")
endif()

if(MINGW AND NOT ${LTO_STRATEGY} STREQUAL "Disabled")
    message(WARNING "LTO strategy under MinGW is still not available. Forcing to Disabled state")
    set(LTO_STRATEGY "Disabled" CACHE STRING "Choose the Link Time Optimization (LTO) strategy, options are: Auto (i.e., optimization is enabled only in release build) Enabled Disabled." FORCE)
endif()

# create a bool cached variable EXPSYMBOLS_STATIC_LINKING set to true if
# the mimic library is going to be delivered static (*.a, *.lib). This will propagate
# to all executables embedding libmimic the information to link the mimic library
# (and mimic library only) with the linker option Wl,--whole-archive.
# This will let the executable to forcefully load symbols from static library
# and fill correctly the factory for tracking mimic blocks.
# This is NOT NEEDED in shared mimic versions.
if(NOT ${BUILD_SHARED_LIBS})
    set(EXPSYMBOLS_STATIC_LINKING 1 CACHE INTERNAL "Build Shared Libraries")
else()
    set(EXPSYMBOLS_STATIC_LINKING 0 CACHE INTERNAL "Build Shared Libraries")
endif()

#------------------------------------------------------------------------------------#
# Set up
#------------------------------------------------------------------------------------#
#let cmake knows the location of custom functions needed by mimic project.
LIST(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/project")

#include preprocessor definitions functions and
#initialize MIMIC_DEFINITIONS_PUBLIC variable.
include(preprocDefinitionsFunctions) #-->coming with mimic, inside MIMIC_CMAKE_DIR/project
initializePublicDefinitions()

# pass precomp variable to control pure API or as-executable distro.
if(BUILD_MIMIC_API_MODE)
    addPrivateDefinitions("MIMIC_API_MODE=1")
else()
    addPrivateDefinitions("MIMIC_API_MODE=0")
endif()
#include Link Time Optimization (LTO) functions and
#check if the Link Time Optimization is usable in this context
include(LTOFunctions) #-->coming with mimic, inside MIMIC_CMAKE_DIR/project
if(DEFINED LTO_STRATEGY)
    checkLTOStrategy(${LTO_STRATEGY} ENABLE_LTO ${BUILD_SHARED_LIBS})
else()
    set(ENABLE_LTO FALSE)
endif()

#include macro to get version of mimic library and use
# library version -> search in the library header file and retrieve
# MIMIC_VERSION, MIMIC_VERSION_MAJOR, MIMIC_VERSION_MINOR, MIMIC_VERSION_PATCH and
# MIMIC_VERSION_TAG
include(libraryMacro) #-->coming with mimic, inside MIMIC_CMAKE_DIR/project
set (VERSION_HEADER_FILENAME "${PROJECT_SOURCE_DIR}/src/common/${CMAKE_PROJECT_NAME}_version.hpp")
libraryVersion(${VERSION_HEADER_FILENAME}) #-->coming with mimic, inside MIMIC_CMAKE_DIR/project

#include compilerSettings functions and customize compilers for mimic object
include(compilerSettings) #-->coming with mimic, inside MIMIC_CMAKE_DIR/project
# the method will require CMAKE_BUILD_TYPE, VERBOSE_MAKE and ENABLE_MPI to be defined.
setCompilers()

#------------------------------------------------------------------------------------#
# add MPI compiler flags - if ENABLE_MPI is active
# if a MPI mimic is used, MPI package is automatically searched by mimic's sub-dep bitpit.
# MPI_CXX_COMPILE_FLAGS and MPI_CXX_LINK_FLAGS are already available if mimic-bitpit and their
# chain of deps are correctly found.
#------------------------------------------------------------------------------------#
if (ENABLE_MPI)
    if(MPI_CXX_COMPILE_FLAGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILE_FLAGS}")
    endif()

    if(MPI_CXX_LINK_FLAGS)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MPI_CXX_LINK_FLAGS}")
    endif()
endif()

#------------------------------------------------------------------------------------#
# List of all available modules
#------------------------------------------------------------------------------------#
set(MIMIC_MODULE_LIST "")
list(APPEND MIMIC_MODULE_LIST "common")
list(APPEND MIMIC_MODULE_LIST "core")
list(APPEND MIMIC_MODULE_LIST "io")
list(APPEND MIMIC_MODULE_LIST "generators")
list(APPEND MIMIC_MODULE_LIST "iocustomdata")
list(APPEND MIMIC_MODULE_LIST "ioofoam")
list(APPEND MIMIC_MODULE_LIST "iocgns")
list(APPEND MIMIC_MODULE_LIST "manipulators")
list(APPEND MIMIC_MODULE_LIST "laplacians")
list(APPEND MIMIC_MODULE_LIST "propagators")
list(APPEND MIMIC_MODULE_LIST "deprecated")
list(APPEND MIMIC_MODULE_LIST "vtkwrappers")
list(APPEND MIMIC_MODULE_LIST "generativedesign")
list(APPEND MIMIC_MODULE_LIST "levelset")
list(APPEND MIMIC_MODULE_LIST "geohandlers")
list(APPEND MIMIC_MODULE_LIST "utils")
list(APPEND MIMIC_MODULE_LIST "nurbs")
list(APPEND MIMIC_MODULE_LIST "objwavefront")
list(APPEND MIMIC_MODULE_LIST "wrapperblocks")
list(APPEND MIMIC_MODULE_LIST "gui")

#------------------------------------------------------------------------------------#
# Modules inter-dependecies
#------------------------------------------------------------------------------------#
set(COMMON_DEPS "")
set(CORE_DEPS "common")
set(IO_DEPS "core")
set(GENERATORS_DEPS "io")
#this 4 are always in

set(IOCUSTOMDATA_DEPS "generators")
set(IOOFOAM_DEPS "generators")
set(IOCGNS_DEPS "generators")
set(LAPLACIANS_DEPS "generators")
set(PROPAGATORS_DEPS "generators")
set(DEPRECATED_DEPS "manipulators")
set(VTKWRAPPERS_DEPS "generators")
set(MANIPULATORS_DEPS "geohandlers")
set(GENERATIVEDESIGN_DEPS "generators;levelset;geohandlers")
set(LEVELSET_DEPS "core")
set(GEOHANDLERS_DEPS "generators")
set(UTILS_DEPS "generators")
set(NURBS_DEPS "utils")
set(OBJWAVEFRONT_DEPS "geohandlers")
set(WRAPPERBLOCKS_DEPS "generators;geohandlers;laplacians")

set(GUI_DEPS "generators")


#------------------------------------------------------------------------------------#
# Modules optional external dependencies
#------------------------------------------------------------------------------------#
if(BUILD_MIMIC_API_MODE)
    set(COMMON_EXTERNAL_DEPS "")
else()
    set(COMMON_EXTERNAL_DEPS "NETLICENSING-CPP")
endif()

set(IO_EXTERNAL_DEPS "")
set(GENERATORS_EXTERNAL_DEPS "")
set(IOCUSTOMDATA_EXTERNAL_DEPS "")
set(IOCGNS_EXTERNAL_DEPS "CGNS")
set(IOOFOAM_EXTERNAL_DEPS "OPENFOAM")
set(LAPLACIANS_EXTERNAL_DEPS "")
set(PROPAGATORS_EXTERNAL_DEPS "")
set(DEPRECATED_EXTERNAL_DEPS "")
set(VTKWRAPPERS_EXTERNAL_DEPS "VTK")
set(MANIPULATORS_EXTERNAL_DEPS "")
set(GENERATIVEDESIGN_EXTERNAL_DEPS "")
set(GEOHANDLERS_EXTERNAL_DEPS "")
set(UTILS_EXTERNAL_DEPS "")
set(NURBS_EXTERNAL_DEPS "")
set(OBJWAVEFRONT_EXTERNAL_DEPS "")
set(WRAPPERBLOCKS_EXTERNAL_DEPS "")

if(GENERATIVEDESIGN_ENABLE_OMP)
    set(LEVELSET_EXTERNAL_DEPS "OpenMP")
else()
    set(LEVELSET_EXTERNAL_DEPS "")
endif()

if(GENERATIVEDESIGN_ENABLE_VTK)
    list(APPEND GENERATIVEDESIGN_DEPS "vtkwrappers")
    list(APPEND GENERATIVEDESIGN_EXTERNAL_DEPS "VTK")
else()
    list(REMOVE_ITEM GENERATIVEDESIGN_DEPS "vtkwrappers")
    list(REMOVE_ITEM GENERATIVEDESIGN_EXTERNAL_DEPS "VTK")
endif()

set(GUI_EXTERNAL_DEPS "")

## create a list of all possible deps in the project - this will be used
## inside MIMICConfig.cmake to build an automatic cleaning of unused cache variables.
list(APPEND MIMIC_EXTERNAL_POSSIBLE_DEPS)
foreach(MODULE_NAME IN LISTS MIMIC_MODULE_LIST)
    string(TOUPPER ${MODULE_NAME} UPPER_MODULE_NAME)
    list(APPEND MIMIC_EXTERNAL_POSSIBLE_DEPS ${${UPPER_MODULE_NAME}_EXTERNAL_DEPS})
endforeach()

#------------------------------------------------------------------------------------#
# Experimental/deprecated features
#------------------------------------------------------------------------------------#
include(expFeatureFunctions)
#
# to be assessed and used in the next feature
#



#------------------------------------------------------------------------------------#
# Check of inter-module dependencies
#------------------------------------------------------------------------------------#
#include module handle functions
include(moduleHandleFunctions)

#------------------------------------------------------------------------------------#
# Force enabling of the basic modules
#------------------------------------------------------------------------------------#
# The modules 'common', 'core', 'io', 'generators' have to be always built - mimic mainframe
getModuleEnableName("common" COMMON_MODULE_ENABLED)
unset(${COMMON_MODULE_ENABLED} CACHE)
set(${COMMON_MODULE_ENABLED} 1 CACHE INTERNAL "Request building common module" FORCE)

getModuleEnableName("core" CORE_MODULE_ENABLED)
unset(${CORE_MODULE_ENABLED} CACHE)
set(${CORE_MODULE_ENABLED} 1 CACHE INTERNAL "Request building core module" FORCE)

getModuleEnableName("io" IO_MODULE_ENABLED)
unset(${IO_MODULE_ENABLED} CACHE)
set(${IO_MODULE_ENABLED} 1 CACHE INTERNAL "Request building io module" FORCE)

getModuleEnableName("generators" GENERATORS_MODULE_ENABLED)
unset(${GENERATORS_MODULE_ENABLED} CACHE)
set(${GENERATORS_MODULE_ENABLED} 1 CACHE INTERNAL "Request building generators module" FORCE)


##OTHER MODULES TO BE OPTIONAL SET ACTIVE OR UNACTIVE

# list of all enabled by default
set(MIMIC_MODULEENABLED_LIST "")
list(APPEND MIMIC_MODULEENABLED_LIST "manipulators")
list(APPEND MIMIC_MODULEENABLED_LIST "laplacians")
list(APPEND MIMIC_MODULEENABLED_LIST "propagators")
list(APPEND MIMIC_MODULEENABLED_LIST "geohandlers")
list(APPEND MIMIC_MODULEENABLED_LIST "utils")

# list of all disabled by default
set(MIMIC_MODULEDISABLED_LIST "")
list(APPEND MIMIC_MODULEDISABLED_LIST "nurbs")
list(APPEND MIMIC_MODULEDISABLED_LIST "vtkwrappers")
list(APPEND MIMIC_MODULEDISABLED_LIST "generativedesign")
list(APPEND MIMIC_MODULEDISABLED_LIST "levelset")
list(APPEND MIMIC_MODULEDISABLED_LIST "iocustomdata")
list(APPEND MIMIC_MODULEDISABLED_LIST "iocgns")
list(APPEND MIMIC_MODULEDISABLED_LIST "ioofoam")
list(APPEND MIMIC_MODULEDISABLED_LIST "objwavefront")
list(APPEND MIMIC_MODULEDISABLED_LIST "deprecated")
list(APPEND MIMIC_MODULEDISABLED_LIST "wrapperblocks")
list(APPEND MIMIC_MODULEDISABLED_LIST "gui")

# Initialize module state
foreach (MODULE_NAME IN LISTS MIMIC_MODULEENABLED_LIST)
    enableModule(${MODULE_NAME} TRUE FALSE)
endforeach ()

foreach (MODULE_NAME IN LISTS MIMIC_MODULEDISABLED_LIST)
    enableModule(${MODULE_NAME} FALSE FALSE)
endforeach ()

set(CHECK_DEPS 1)
while (${CHECK_DEPS} EQUAL 1)
    set(CHECK_DEPS 0)
    foreach (MODULE_NAME IN LISTS MIMIC_MODULE_LIST)
        isModuleEnabled(${MODULE_NAME} MODULE_ENABLED)
        if (MODULE_ENABLED)
            string(TOUPPER ${MODULE_NAME} UPPER_MODULE_NAME)
            foreach (DEPENDENCY_NAME IN LISTS ${UPPER_MODULE_NAME}_DEPS)
                isModuleEnabled(${DEPENDENCY_NAME} DEPENDENCY_ENABLED)
                if (NOT DEPENDENCY_ENABLED)
                    set(CHECK_DEPS 1)
                    enableModule(${DEPENDENCY_NAME} TRUE TRUE)
                    message("Module '${DEPENDENCY_NAME}' has been enabled because it's needed by module '${MODULE_NAME}'")
                endif ()
            endforeach ()
        endif ()
        unset(MODULE_ENABLE_FLAG)
    endforeach ()
endwhile()
unset(CHECK_DEPS)

# disabling ioofoam module in Windows-Mingw compilation.
if(MINGW)
    getModuleEnableName("ioofoam" IOOFOAM_MODULE_ENABLED)
    unset(${IOOFOAM_MODULE_ENABLED} CACHE)
    set(${IOOFOAM_MODULE_ENABLED} 0 CACHE INTERNAL "Request building ioofoam module" FORCE)
    message("IOOFOAM module forcefully disabled in Win-Mingw compilation")
endif()

# enable/disable leveset module and expose/hide OMP use flag if generative design module is enabled/disabled
isModuleEnabled("generativedesign" GENERATIVEDESIGN_MODULE_ENABLED)
if (GENERATIVEDESIGN_MODULE_ENABLED)
    getModuleEnableName("levelset" LEVELSET_MODULE_ENABLED)
    unset(${LEVELSET_MODULE_ENABLED} CACHE)
    set(${LEVELSET_MODULE_ENABLED} 1 CACHE INTERNAL "Request building levelset module" FORCE)
    set(GENERATIVEDESIGN_ENABLE_OMP 0 CACHE BOOL "If set, the generative design module is compiled with intel multithread OMP support")
    set(GENERATIVEDESIGN_ENABLE_VTK 0 CACHE BOOL "If set, the generative design module is compiled with VTK support")
else()
    getModuleEnableName("levelset" LEVELSET_MODULE_ENABLED)
    unset(${LEVELSET_MODULE_ENABLED} CACHE)
    set(${LEVELSET_MODULE_ENABLED} 0 CACHE INTERNAL "Request building levelset module" FORCE)
    unset(GENERATIVEDESIGN_ENABLE_OMP CACHE)
    unset(GENERATIVEDESIGN_ENABLE_VTK CACHE)
endif()

# enable the definition of number of threads if OMP support is enabled
if (GENERATIVEDESIGN_ENABLE_OMP)
    set(MIMIC_NUM_THREADS 8 CACHE STRING "Number of threads used during mimic mutlithreads functions execution")
else()
    unset(MIMIC_NUM_THREADS CACHE)
endif()

# add public definitions of variables if omp enabled for generative design module
if (GENERATIVEDESIGN_ENABLE_OMP)
    addPublicDefinitions("${PROJ_NAME}_GENERATIVEDESIGN_ENABLE_OMP=1")
    addPublicDefinitions("MIMIC_NUM_THREADS=${MIMIC_NUM_THREADS}")
else()
    addPublicDefinitions("${PROJ_NAME}_GENERATIVEDESIGN_ENABLE_OMP=0")
    addPublicDefinitions("MIMIC_NUM_THREADS=0")
endif()

# add public definitions of variables if vtk enabled for generative design module
if (GENERATIVEDESIGN_ENABLE_VTK)
    addPublicDefinitions("${PROJ_NAME}_GENERATIVEDESIGN_ENABLE_VTK=1")
else()
    addPublicDefinitions("${PROJ_NAME}_GENERATIVEDESIGN_ENABLE_VTK=0")
endif()

#temporary in BETA_NOGUI version
hideModule("gui")

#------------------------------------------------------------------------------------#
# External modules dependecies
#------------------------------------------------------------------------------------#

set(MIMIC_EXTERNAL_DEPENDENCIES "") ## put here the name of package you need to refind with main FindMimic
set(MIMIC_EXTERNAL_LIBRARIES "") ## put here the xxx_LIBRARIES you need now.
set(MIMIC_EXTERNAL_INCLUDE_DIRS "") ## put here the xxx_INCLUDE_DIRS you need now.

set(MIMIC_PROTECTED_LIBRARIES "") ## put here the xxx_LIBRARIES you don't want to refind with findPackage once mimic is installed.
set(MIMIC_PROTECTED_INCLUDE_DIRS "") ## put here the xxx_INCLUDE_DIRS you don't want to refind with findPackage once mimic is installed

##list now the really needed deps required by the ACTIVE modules
set(EXTERNAL_ACTIVE_DEPS "")
foreach(MODULE_NAME IN LISTS MIMIC_MODULE_LIST)
    isModuleEnabled(${MODULE_NAME} MODULE_ENABLED)
    string(TOUPPER ${MODULE_NAME} UPPER_MODULE_NAME)
    if(MODULE_ENABLED)
        list(APPEND EXTERNAL_ACTIVE_DEPS ${${UPPER_MODULE_NAME}_EXTERNAL_DEPS})
    endif()
endforeach()

#include utilities for cached variables cleaners
include(cacheVarCleaners)

###     BITPIT      ############################################################
#fill the bitpit module list required by mimic.
set(BITPIT_QUERYPACKAGES "levelset" "surfunstructured" "volunstructured" "RBF" "CG" "volcartesian" "voloctree")
if(MODULE_ENABLED_PROPAGATORS)
    list (APPEND BITPIT_QUERYPACKAGES "discretization")
endif()
if(MODULE_ENABLED_LAPLACIANS)
    list (APPEND BITPIT_QUERYPACKAGES "LA")
endif()

enable_language(C)
# some features of findBitpit will require c language to work properly
find_package(BITPIT REQUIRED COMPONENTS ${BITPIT_QUERYPACKAGES})
include(${BITPIT_USE_FILE})

list (APPEND MIMIC_EXTERNAL_DEPENDENCIES "BITPIT")
list (APPEND MIMIC_EXTERNAL_LIBRARIES "${BITPIT_LIBRARIES}")
# include dirs are managed with BITPIT_USE_FILE.

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
################################################################################

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
################################################################################

### CGNS #######################################################################
list(FIND EXTERNAL_ACTIVE_DEPS "CGNS" _CGNS_index)
if (${_CGNS_index} GREATER -1)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/cgns/cmake")

    find_package(CGNS REQUIRED)

    list (APPEND MIMIC_EXTERNAL_DEPENDENCIES "CGNS")
    list (APPEND MIMIC_EXTERNAL_LIBRARIES "${CGNS_LIBRARIES}")
    list (APPEND MIMIC_EXTERNAL_INCLUDE_DIRS "${CGNS_INCLUDE_DIRS}")

else()
    cleanSingleCacheVariables("CGNS")
endif ()

unset(_CGNS_index)
################################################################################

### METIS ###########################################################
#---NOT USED NOW. BITPIT MPI IS SEARCHING IT.
# list(FIND EXTERNAL_ACTIVE_DEPS "METIS" _METIS_index)
# if (${_METIS_index} GREATER -1)
#      list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/metis/cmake")

#      find_package(METIS REQUIRED)

#      list (APPEND MIMIC_EXTERNAL_DEPENDENCIES "METIS")
#      list (APPEND MIMIC_EXTERNAL_LIBRARIES "${METIS_LIBRARIES}")
#      list (APPEND MIMIC_EXTERNAL_INCLUDE_DIRS "${METIS_INCLUDE_DIRS}")

# else()
#     cleanMultiCacheVariables("METIS")
# endif ()

# unset(_METIS_index)
################################################################################

### parMETIS ###########################################################
#---NOT USED NOW. IN CASE STASH METIS SEARCH AND USE THE RIGHT BELOW ---
#--- IT WILL SEARCH FOR BOTH METIS AND PARMETIS
# list(FIND EXTERNAL_ACTIVE_DEPS "PARMETIS" _PARMETIS_index)
# if (${_PARMETIS_index} GREATER -1)
#      list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/parmetis/cmake")
#      list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/metis/cmake")

#      find_package(PARMETIS REQUIRED)

#      list (APPEND MIMIC_EXTERNAL_DEPENDENCIES "PARMETIS")
#      list (APPEND MIMIC_EXTERNAL_LIBRARIES "${PARMETIS_LIBRARIES}")
#      list (APPEND MIMIC_EXTERNAL_INCLUDE_DIRS "${PARMETIS_INCLUDE_DIRS}")

# else()
#     cleanMultiCacheVariables("PARMETIS;METIS")
# endif ()

# unset(_PARMETIS_index)
################################################################################


###    VTK      #################################################################
## NFDev: supporting also versions from 8 to 9 for now, for operational purposes.
## To be addressed exclusively to version > 9.0.0 in the next future.
list(FIND EXTERNAL_ACTIVE_DEPS "VTK" _VTK_index)
if (${_VTK_index} GREATER -1)
    set(VTK_OLDER_DISTRO 0 CACHE BOOL "set to true if VTK distro in use is < 9.0.0")

    if(VTK_OLDER_DISTRO)
        set(VTK_QUERYPACKAGES "vtkCommonDataModel" "vtkCommonCore" "vtkFiltersCore" "vtkFiltersGeneral" "vtkFiltersGeometry" "vtkIOXML" "vtkIOLegacy" "vtkFiltersPoints" "vtkIOGeometry")
    else()
        set(VTK_QUERYPACKAGES "CommonDataModel" "CommonCore" "FiltersCore" "FiltersGeneral" "FiltersGeometry" "IOXML" "IOLegacy" "IOGeometry")
    endif()

    find_package(VTK REQUIRED COMPONENTS ${VTK_QUERYPACKAGES})

    list (APPEND MIMIC_EXTERNAL_DEPENDENCIES "VTK")

    if(VTK_OLDER_DISTRO)
        include(${VTK_USE_FILE})
        addPublicDefinitions("VTK_OLDER_DISTRO=1")
    else()
        # VTK_INCLUDE_DIRS and VTK_DEFINITIONS are no more present in vtk 9.0 or newer versions.
        addPublicDefinitions("VTK_OLDER_DISTRO=0")
    endif()
    #beware VTK will make obsolete also this VTK_LIBRARIES
    #in fact VTK_LIBRARIES is a collection of Module names VTK::CommonDataModel VTK::CommonCore.
    list (APPEND MIMIC_EXTERNAL_LIBRARIES ${VTK_LIBRARIES})

else()
    cleanSingleCacheVariables("VTK")
endif()

unset(_VTK_index)
#############################################################################

### OpenMP dependency
list(FIND EXTERNAL_ACTIVE_DEPS "OpenMP" _OpenMP_index)
if (${_OpenMP_index} GREATER -1)

    find_package(OpenMP REQUIRED)

    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    list (APPEND MIMIC_EXTERNAL_DEPENDENCIES "OpenMP")
    list (APPEND MIMIC_EXTERNAL_LIBRARIES "${OpenMP_CXX_LIBRARIES}")
    list (APPEND MIMIC_EXTERNAL_INCLUDE_DIRS "${OpenMP_CXX_INCLUDE_DIRS}")
else()
    cleanSingleCacheVariables("OpenMP")
endif()
unset(_OpenMP_index)


#############################################################################

###  NetLicensing ###########################################################
list(FIND EXTERNAL_ACTIVE_DEPS "NETLICENSING-CPP" _NETLICENSING-CPP_index)
if (${_NETLICENSING-CPP_index} GREATER -1)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/netlicensing-cpp/cmake")

    find_package(NETLICENSING-CPP REQUIRED)

    list (APPEND MIMIC_EXTERNAL_DEPENDENCIES "NETLICENSING-CPP")
    list (APPEND MIMIC_EXTERNAL_LIBRARIES "${NETLICENSING-CPP_LIBRARIES}")
    list (APPEND MIMIC_EXTERNAL_INCLUDE_DIRS "${NETLICENSING-CPP_INCLUDE_DIRS}")

    addPrivateDefinitions(${NETLICENSING-CPP_DEFINITIONS})

else()
    cleanSingleCacheVariables("NETLICENSING-CPP")
endif ()


## pass now MIMIC_EXTERNAL_INCLUDE_DIRS to the include_directories
include_directories(${MIMIC_EXTERNAL_INCLUDE_DIRS})

#------------------------------------------------------------------------------------#
# Set programming languages
#------------------------------------------------------------------------------------#
get_property(MIMIC_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)

#------------------------------------------------------------------------------------#
# Subdirectories
#------------------------------------------------------------------------------------#

# Main library
add_subdirectory(src)

# Docs
add_subdirectory(doc)

# CMake
add_subdirectory(cmake)

# External
add_subdirectory(external)

# xml text unit interface binaries
add_subdirectory(binaries)

# Examples
add_subdirectory(examples)

# Tests
enable_testing()
add_subdirectory(test)

# Trial serial key utilities
add_subdirectory(trials)


find_package(Boost COMPONENTS filesystem atomic REQUIRED)

add_executable(main main.cpp)
add_executable(Hello2 Hello2.cpp)

target_link_libraries(main PRIVATE ${Boost_LIBRARIES})

install(TARGETS main)

enable_testing()
add_test(NAME main COMMAND main)
add_test(NAME hello2 COMMAND Hello2)

