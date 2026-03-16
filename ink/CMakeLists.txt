add_library(ink ink.cpp)

# ----------------------------------------------
# specify other sources for the ink library
# (if any) with target_sources
# ----------------------------------------------

# include OpenTimer
target_include_directories(ink
  PUBLIC ${INK_3RD_PARTY_DIR}/OpenTimer)

# include moodycamel concurrent queue
target_include_directories(ink
  PUBLIC ${INK_3RD_PARTY_DIR}/concurrentqueue)

# include rigtorp concurrent queue
target_include_directories(ink
  PUBLIC ${INK_3RD_PARTY_DIR}/MPMCQueue/include/rigtorp)

# include oneTBB library headers
# On macOS we use the system/Homebrew TBB, so skip the submodule include path.
if(NOT APPLE)
  target_include_directories(ink
    PUBLIC ${INK_3RD_PARTY_DIR}/oneTBB/include)
endif()

# include taskflow
#target_include_directories(ink
#  PUBLIC ${INK_3RD_PARTY_DIR}/taskflow)


# find the pthreads library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Filesystem REQUIRED)

target_link_libraries(ink PRIVATE Threads::Threads)
# target_link_libraries(ink PRIVATE CUDA::cudart)
target_link_libraries(ink PUBLIC OpenTimer)
target_link_libraries(ink PUBLIC std::filesystem)


# On macOS, Apple Clang does not ship with OpenMP; use Homebrew libomp.
if(APPLE)
  execute_process(
    COMMAND brew --prefix libomp
    OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
  if(HOMEBREW_LIBOMP_PREFIX)
    set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
    set(OpenMP_C_LIB_NAMES "omp")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY "${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib")
  endif()
endif()

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ink PUBLIC OpenMP::OpenMP_CXX)
endif()

# find oneTBB library
find_package(TBB REQUIRED)
target_link_libraries(ink PUBLIC TBB::tbb)



target_compile_options(ink INTERFACE -Wall -Wextra -Wfatal-errors)

# On macOS, libc++ requires this flag and the experimental library to expose
# parallel STL policies (std::execution::par, par_unseq).
if(APPLE)
  target_compile_definitions(ink PUBLIC _LIBCPP_ENABLE_EXPERIMENTAL)
  target_link_libraries(ink PUBLIC
    ${CMAKE_OSX_SYSROOT}/usr/lib/libc++experimental.a)
endif()


