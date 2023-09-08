# example: simple.cpp
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/simple)
# add_executable(simple_timing ${INK_EXAMPLE_DIR}/simple/simple.cpp)
 
# example: c432.cpp
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c432)
# add_executable(c432_timing ${INK_EXAMPLE_DIR}/c432/c432.cpp)
 
# example: vga_lcd.cpp
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/vga_lcd)
# add_executable(vga_lcd_timing ${INK_EXAMPLE_DIR}/vga_lcd/vga_lcd.cpp)

# example: tv80.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/tv80)
add_executable(tv80_example ${INK_EXAMPLE_DIR}/tv80/tv80.cpp)

# example: wb_dma.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/wb_dma)
add_executable(wb_dma_example ${INK_EXAMPLE_DIR}/wb_dma/wb_dma.cpp)

# example: des_perf.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/des_perf)
add_executable(des_perf_example ${INK_EXAMPLE_DIR}/des_perf/des_perf.cpp)

# example: des_perf.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/aes_core)
add_executable(aes_core_example ${INK_EXAMPLE_DIR}/aes_core/aes_core.cpp)

list(APPEND INK_EXAMPLES
  tv80_example
  wb_dma_example
  des_perf_example
  aes_core_example
)

find_package(Threads REQUIRED)
foreach(eg IN LISTS INK_EXAMPLES) 
  # link external libaries
  target_link_libraries(${eg}
    PRIVATE Threads::Threads
    PUBLIC OpenTimer ink)

  # include OpenTimer
  target_include_directories(${eg}
    PUBLIC ${INK_3RD_PARTY_DIR}/OpenTimer)
endforeach()


