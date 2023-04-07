set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INKTIMER_EXAMPLE_DIR}/simple)
add_executable(simple_app ${INKTIMER_EXAMPLE_DIR}/simple/simple.cpp)



find_package(Threads REQUIRED)
# link external libaries
target_link_libraries(simple_app
  PRIVATE Threads::Threads
  PUBLIC OpenTimer stdc++fs
)


# include OpenTimer
target_include_directories(simple_app
  PUBLIC ${INKTIMER_3RD_PARTY_DIR}/OpenTimer)


