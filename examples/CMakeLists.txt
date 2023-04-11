
# example: simple.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/simple)
add_executable(simple_app ${INK_EXAMPLE_DIR}/simple/simple.cpp)

# example: incremental.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/incremental)
add_executable(inc_app ${INK_EXAMPLE_DIR}/incremental/incremental.cpp)


list(APPEND INK_EXAMPLES
  simple_app
  inc_app
)

find_package(Threads REQUIRED)
foreach(eg IN LISTS INK_EXAMPLES) 
  # link external libaries
  target_link_libraries(${eg}
    PRIVATE Threads::Threads
    PUBLIC OpenTimer stdc++fs ink)


  # include OpenTimer
  target_include_directories(${eg}
    PUBLIC ${INK_3RD_PARTY_DIR}/OpenTimer)

endforeach()


