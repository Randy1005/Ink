add_library(ink ink.cpp)

# ----------------------------------------------
# specify other sources for the ink library
# (if any) with target_sources
# ----------------------------------------------

# include OpenTimer
target_include_directories(ink
  PUBLIC ${INK_3RD_PARTY_DIR}/OpenTimer)



# find the pthreads library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(ink PRIVATE Threads::Threads)
target_link_libraries(ink PRIVATE CUDA::cudart)
target_link_libraries(ink PUBLIC OpenTimer)
target_link_libraries(ink PUBLIC stdc++fs)



set_property(TARGET ink PROPERTY CXX_STANDARD 23)
target_compile_options(ink INTERFACE -Wall -Wextra -Wfatal-errors)


