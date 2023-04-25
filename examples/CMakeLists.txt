
# example: simple.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/simple)
add_executable(simple_timing ${INK_EXAMPLE_DIR}/simple/simple.cpp)

# example: incremental.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/incremental)
add_executable(inc_timing ${INK_EXAMPLE_DIR}/incremental/incremental.cpp)

# example: c17.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c17)
add_executable(c17_timing ${INK_EXAMPLE_DIR}/c17/c17.cpp)


# example: s27.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/s27)
add_executable(s27_timing ${INK_EXAMPLE_DIR}/s27/s27.cpp)

# example: c3_slack.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c3_slack)
add_executable(c3_slack_timing ${INK_EXAMPLE_DIR}/c3_slack/c3_slack.cpp)

# example: c17_slack.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c17_slack)
add_executable(c17_slack_timing ${INK_EXAMPLE_DIR}/c17_slack/c17_slack.cpp)


# example: c17_slack.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c432)
add_executable(c432_timing ${INK_EXAMPLE_DIR}/c432/c432.cpp)


list(APPEND INK_EXAMPLES
  simple_timing
  inc_timing
  c17_timing
  s27_timing
  c3_slack_timing
  c17_slack_timing
  c432_timing
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


