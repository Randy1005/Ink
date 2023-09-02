cmake_minimum_required(VERSION 3.18.0)

# this is hardcoded, can I configure cmake to auto locate nvcc for me?
# set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)


# project(Ink VERSION 1.0 LANGUAGES C CXX CUDA)
project(Ink VERSION 1.0 LANGUAGES C CXX)


# set everything up for c++XX features
set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CUDA_ARCHITECTURES 75)



#------------------------------------------------------------------------------
# default release build
#------------------------------------------------------------------------------

# set compilation flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to Release ")
  set(
    CMAKE_BUILD_TYPE Release
    CACHE
    STRING "Choose the type of build."
    FORCE
  )
  # Set the possible values of build type for cmake-gui
  set_property(
    CACHE
    CMAKE_BUILD_TYPE
    PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )

endif()

# set(CMAKE_CUDA_FLAGS_DEBUG "-g -G -Xcompiler -g -lineinfo --expt-extended-lambda")

# Turn on the verbose
set(CMAKE_VERBOSE_MAKEFILE ON)


# Library-specific variable
set(INK_3RD_PARTY_DIR ${PROJECT_SOURCE_DIR}/3rd-party)
set(INK_EXAMPLE_DIR ${PROJECT_SOURCE_DIR}/examples)

# Benchmark directory variable
set(INK_BENCHMARK_DIR ${PROJECT_SOURCE_DIR}/benchmarks)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)


# Binary folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
message("PROJECT_NAME: " ${PROJECT_NAME})
message("INK_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message(STATUS "INK_BENCHMARK_DIR: " ${INK_BENCHMARK_DIR})
message(STATUS "BIN_DIR: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# test that filesystem header actually is there and works
try_compile(HAS_FS "${CMAKE_BINARY_DIR}/temp" 
"${CMAKE_SOURCE_DIR}/tests/has_filesystem.cc" 
            CMAKE_FLAGS -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON
            LINK_LIBRARIES stdc++fs)
if(HAS_FS)
    message(STATUS "Compiler has filesystem support")
else()
#   .... You could also try searching for boost::filesystem here.
    message(STATUS "Compiler is missing filesystem capabilities")
endif(HAS_FS)




option(INK_BUILD_CUDA "Enables builds of cuda code" OFF)



# -----------------------------------------------------------------------------
# must-have package include
# -----------------------------------------------------------------------------
include(CheckLanguage)

# Enable test
include(CTest)
set(CMAKE_CTEST_ARGUMENTS "--output-on-failure")



# -----------------------------------------------------------------------------
# find nvcc
# https://cliutils.gitlab.io/modern-cmake/chapters/packages/CUDA.html
# -----------------------------------------------------------------------------
if(INK_BUILD_CUDA)
  message(STATUS "Configuring CUDA ...")
  check_language(CUDA)
  if(NOT CMAKE_CUDA_COMPILER)
    message(FATAL_ERROR "\nNo CUDA compiler found")
  endif()
  enable_language(CUDA)
  find_package(CUDAToolkit REQUIRED)
  add_definitions(-DUSE_CUDA)
  include_directories("${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")
  
  if(CMAKE_CUDA_COMPILER_VERSION VERSION_LESS "11")
    message(STATUS "CMAKE_CUDA_COMPILER_VERSION: ${CMAKE_CUDA_COMPILER_VERSION}")
    message(FATAL_ERROR "\nQSat requires CUDA at least v11")
  endif()
endif()  


# -----------------------------------------------------------------------------
# include sub cmake list
# -----------------------------------------------------------------------------

# add sub directories
add_subdirectory(ink)
add_subdirectory(${INK_3RD_PARTY_DIR}/PEGTL)
add_subdirectory(${INK_3RD_PARTY_DIR}/OpenTimer)
add_subdirectory(${INK_EXAMPLE_DIR})
add_subdirectory(main)
add_subdirectory(unittests)




# -----------------------------------------------------------------------------
# benchmark directories
# -----------------------------------------------------------------------------


