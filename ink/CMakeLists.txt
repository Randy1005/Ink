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

# include oneTBB library
target_include_directories(ink
  PUBLIC ${INK_3RD_PARTY_DIR}/oneTBB/include)

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

# find oneTBB library
find_package(TBB REQUIRED)
target_link_libraries(ink PUBLIC TBB::tbb)



target_compile_options(ink INTERFACE -Wall -Wextra -Wfatal-errors)


