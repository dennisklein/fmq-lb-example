cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(fmq-lb-example VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CONFIGURATION_TYPES "RelWithDebInfo")
set(CMAKE_CXX_FLAGS_RelWithDebInfo "-O2 -g -Wshadow -Wall -Wextra -Wpedantic -DNDEBUG")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

include(GNUInstallDirs)

# TODO Refactor to use FairFindPackage2 module
find_package(FairMQ 1.4 REQUIRED COMPONENTS sdk)
foreach(dep IN LISTS FairMQ_PACKAGE_DEPENDENCIES)
  if(NOT dep STREQUAL FairLogger)
    if(FairMQ_${dep}_COMPONENTS)
      find_package(${dep} ${FairMQ_${dep}_VERSION} COMPONENTS ${FairMQ_${dep}_COMPONENTS})
    else()
      find_package(${dep} ${FairMQ_${dep}_VERSION})
    endif()
    foreach(dep2 IN LISTS ${dep}_PACKAGE_DEPENDENCIES)
      find_package(${dep2} ${${dep}_${dep2}_VERSION})
    endforeach()
  endif()
endforeach()
find_package(FairLogger 1.6 REQUIRED)
foreach(dep IN LISTS FairLogger_PACKAGE_DEPENDENCIES)
  find_package(${dep} ${FairLogger_${dep}_VERSION})
endforeach()

# https://cmake.org/Wiki/CMake_RPATH_handling
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--enable-new-dtags")
  list(APPEND CMAKE_SHARED_LINKER_FLAGS "-Wl,--enable-new-dtags")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_EXE_NAME_PREFIX "fmq-lb-ex")

add_subdirectory(src/app)
