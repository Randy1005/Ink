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

# example: ac97_ctrl.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/ac97_ctrl)
add_executable(ac97_ctrl_example ${INK_EXAMPLE_DIR}/ac97_ctrl/ac97_ctrl.cpp)

# example: netcard.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/netcard)
add_executable(netcard_example ${INK_EXAMPLE_DIR}/netcard/netcard.cpp)

# example: leon2.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/leon2)
add_executable(leon2_example ${INK_EXAMPLE_DIR}/leon2/leon2.cpp)

# example: leon3.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/leon3)
add_executable(leon3_example ${INK_EXAMPLE_DIR}/leon3/leon3.cpp)



# example: c17.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c17)
add_executable(c17_example ${INK_EXAMPLE_DIR}/c17/c17.cpp)


# multi-iteration-experiment: exp.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(exp ${INK_EXAMPLE_DIR}/exp/exp.cpp)

# multi-iteration-experiment: single-iter.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(single-iter ${INK_EXAMPLE_DIR}/exp/single-iter.cpp)

# multi-iteration-experiment: exp.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(parallel ${INK_EXAMPLE_DIR}/exp/parallel.cpp)

# multi-iteration-experiment: exp.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(part_policy ${INK_EXAMPLE_DIR}/exp/part_policy.cpp)

# multi-iteration-experiment: exp.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(acc ${INK_EXAMPLE_DIR}/exp/accuracy.cpp)

# single-iteration-experiment ot
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(single-iter-ot ${INK_EXAMPLE_DIR}/exp/single-iter-ot.cpp)

# single-iteration-experiment pathgen 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
add_executable(single-iter-pathgen ${INK_EXAMPLE_DIR}/exp/single-iter-pathgen.cpp)


# cpathgen
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
add_executable(toy ${INK_EXAMPLE_DIR}/cpathgen/toy.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
add_executable(gen-golden ${INK_EXAMPLE_DIR}/cpathgen/gen-golden.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
add_executable(big-table ${INK_EXAMPLE_DIR}/cpathgen/big-table.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
add_executable(convert-gpathgen-benchmark-to-edge-insertions
  ${INK_EXAMPLE_DIR}/cpathgen/convert-gpathgen-benchmark-to-edge-insertions.cpp)

# speedup-acc-vs-num-threads
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
add_executable(speedup-acc-vs-num-threads
  ${INK_EXAMPLE_DIR}/cpathgen/speedup-acc-vs-num-threads.cpp) 

# runtime-vs-path-cnt
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
add_executable(runtime-vs-path-cnt
  ${INK_EXAMPLE_DIR}/cpathgen/runtime-vs-path-cnt.cpp)

# runtime-steps-vs-delta
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
add_executable(runtime-steps-vs-delta
  ${INK_EXAMPLE_DIR}/cpathgen/runtime-steps-vs-delta.cpp)

# err-vs-path-cnt
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
add_executable(err-vs-path-cnt
  ${INK_EXAMPLE_DIR}/cpathgen/err-vs-path-cnt.cpp)


list(APPEND INK_EXAMPLES
  vga_lcd_example
  tv80_example
  wb_dma_example
  des_perf_example
  aes_core_example
  ac97_ctrl_example
  netcard_example
  c17_example
  leon2_example
  leon3_example
  exp
  single-iter  
  parallel 
  part_policy 
  acc
  single-iter-ot
  single-iter-pathgen
  toy
  gen-golden
  big-table
  convert-gpathgen-benchmark-to-edge-insertions
  speedup-acc-vs-num-threads
  runtime-vs-path-cnt
  runtime-steps-vs-delta
  err-vs-path-cnt
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


