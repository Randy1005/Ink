add_library(inktimer inktimer.cpp)

# ----------------------------------------------
# specify other sources for the inktimer library
# (if any) with target_sources
# ----------------------------------------------

# include OpenTimer
target_include_directories(inktimer
  PUBLIC ${INKTIMER_3RD_PARTY_DIR}/OpenTimer)



# find the pthreads library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(inktimer PRIVATE Threads::Threads)
target_link_libraries(inktimer PRIVATE CUDA::cudart)
target_link_libraries(inktimer PUBLIC OpenTimer)
target_link_libraries(inktimer PUBLIC stdc++fs)



set_property(TARGET inktimer PROPERTY CXX_STANDARD 20)
target_compile_options(inktimer INTERFACE -Wall -Wextra -Wfatal-errors)


