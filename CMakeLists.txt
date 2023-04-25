cmake_minimum_required(VERSION 3.26.0)

# this is hardcoded, can I configure cmake to auto locate nvcc for me?
set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)


project(Ink VERSION 1.0 LANGUAGES C CXX CUDA)


# set everything up for c++XX features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_ARCHITECTURES 75)


# set default compiler to be gcc g++ (DISCOURAGED)
# set(CMAKE_C_COMPILER $ENV{DPCPP_COMPILER})

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

set(CMAKE_CUDA_FLAGS_DEBUG "-g -G -Xcompiler -g -lineinfo --expt-extended-lambda")

# Turn on the verbose
set(CMAKE_VERBOSE_MAKEFILE ON)


# Library-specific variable
set(INK_3RD_PARTY_DIR ${PROJECT_SOURCE_DIR}/3rd-party)
set(INK_EXAMPLE_DIR ${PROJECT_SOURCE_DIR}/examples)

# Benchmark directory variable
set(INK_BENCHMARK_DIR ${PROJECT_SOURCE_DIR}/benchmarks)



# Binary folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
message("PROJECT_NAME: " ${PROJECT_NAME})
message("INK_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message(STATUS "INK_BENCHMARK_DIR: " ${INK_BENCHMARK_DIR})
message(STATUS "BIN_DIR: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})



option(INK_BUILD_CUDA "Enables builds of cuda code" ON)



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
add_subdirectory(inttests)




# -----------------------------------------------------------------------------
# benchmark directories
# -----------------------------------------------------------------------------


