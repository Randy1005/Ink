# example: simple.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/simple)
add_executable(simple_timing ${INK_EXAMPLE_DIR}/simple/simple.cpp)
 
# example: c432.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c432)
add_executable(c432_timing ${INK_EXAMPLE_DIR}/c432/c432.cpp)
 
# example: vga_lcd.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/vga_lcd)
add_executable(vga_lcd_timing ${INK_EXAMPLE_DIR}/vga_lcd/vga_lcd.cpp)

# example: tv80.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/tv80)
add_executable(tv80_timing ${INK_EXAMPLE_DIR}/tv80/tv80.cpp)

# example: c7552.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c7552)
add_executable(c7552_timing ${INK_EXAMPLE_DIR}/c7552/c7552.cpp)

# example: c17.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c17)
add_executable(c17_timing ${INK_EXAMPLE_DIR}/c17/c17.cpp)

list(APPEND INK_EXAMPLES
  simple_timing
  c432_timing
  vga_lcd_timing
  tv80_timing
  c7552_timing
  c17_timing
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


