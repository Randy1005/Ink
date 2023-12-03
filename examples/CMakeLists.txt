# example: vga_lcd.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/vga_lcd)
add_executable(vga_lcd_example ${INK_EXAMPLE_DIR}/vga_lcd/vga_lcd.cpp)

# example: tv80.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/tv80)
add_executable(tv80_example ${INK_EXAMPLE_DIR}/tv80/tv80.cpp)

# example: wb_dma.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/wb_dma)
add_executable(wb_dma_example ${INK_EXAMPLE_DIR}/wb_dma/wb_dma.cpp)

# example: des_perf.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/des_perf)
add_executable(des_perf_example ${INK_EXAMPLE_DIR}/des_perf/des_perf.cpp)

# example: aes_core.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/aes_core)
add_executable(aes_core_example ${INK_EXAMPLE_DIR}/aes_core/aes_core.cpp)

# example: aes_core.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/ac97_ctrl)
add_executable(ac97_ctrl_example ${INK_EXAMPLE_DIR}/ac97_ctrl/ac97_ctrl.cpp)

# example: netcard.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/netcard)
add_executable(netcard_example ${INK_EXAMPLE_DIR}/netcard/netcard.cpp)

# multi-iteration-experiment: exp.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(exp ${INK_EXAMPLE_DIR}/exp/exp.cpp)

# multi-iteration-experiment: single-iter.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(single-iter ${INK_EXAMPLE_DIR}/exp/single-iter.cpp)

list(APPEND INK_EXAMPLES
  vga_lcd_example
  tv80_example
  wb_dma_example
  des_perf_example
  aes_core_example
  ac97_ctrl_example
  netcard_example
  exp
  single-iter  
)

find_package(Threads REQUIRED)
foreach(eg IN LISTS INK_EXAMPLES) 
  # include OpenTimer
  target_include_directories(${eg}
    PUBLIC 
    ${INK_3RD_PARTY_DIR}/OpenTimer
    ${PROJECT_SOURCE_DIR})
  
  # link external libaries
  target_link_libraries(${eg}
    PRIVATE Threads::Threads
    PUBLIC OpenTimer ink)




endforeach()


