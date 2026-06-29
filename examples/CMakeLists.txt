# Helper macro: add an example executable only if its source file exists.
# Usage: ink_add_example(<target-name> <source-file>)
macro(ink_add_example target src)
  if(EXISTS "${src}")
    add_executable(${target} ${src})
    list(APPEND INK_EXAMPLES ${target})
  else()
    message(STATUS "Skipping ${target}: source not found (${src})")
  endif()
endmacro()

# example: vga_lcd.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/vga_lcd)
ink_add_example(vga_lcd_example ${INK_EXAMPLE_DIR}/vga_lcd/vga_lcd.cpp)

# example: tv80.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/tv80)
ink_add_example(tv80_example ${INK_EXAMPLE_DIR}/tv80/tv80.cpp)

# example: wb_dma.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/wb_dma)
ink_add_example(wb_dma_example ${INK_EXAMPLE_DIR}/wb_dma/wb_dma.cpp)

# example: des_perf.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/des_perf)
ink_add_example(des_perf_example ${INK_EXAMPLE_DIR}/des_perf/des_perf.cpp)

# example: aes_core.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/aes_core)
ink_add_example(aes_core_example ${INK_EXAMPLE_DIR}/aes_core/aes_core.cpp)

# example: ac97_ctrl.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/ac97_ctrl)
ink_add_example(ac97_ctrl_example ${INK_EXAMPLE_DIR}/ac97_ctrl/ac97_ctrl.cpp)

# example: netcard_iccad.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/netcard_iccad)
ink_add_example(netcard_example ${INK_EXAMPLE_DIR}/netcard_iccad/netcard_iccad.cpp)

# example: leon2_iccad_eval.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/leon2_iccad_eval)
ink_add_example(leon2_example ${INK_EXAMPLE_DIR}/leon2_iccad_eval/leon2_iccad_eval.cpp)

# example: leon3mp_iccad_eval.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/leon3mp_iccad_eval)
ink_add_example(leon3mp_example ${INK_EXAMPLE_DIR}/leon3mp_iccad_eval/leon3mp_iccad_eval.cpp)

# example: c17.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/c17)
ink_add_example(c17_example ${INK_EXAMPLE_DIR}/c17/c17.cpp)

# multi-iteration-experiment: exp.cpp
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/exp)
ink_add_example(exp            ${INK_EXAMPLE_DIR}/exp/exp.cpp)
ink_add_example(single-iter    ${INK_EXAMPLE_DIR}/exp/single-iter.cpp)
ink_add_example(parallel       ${INK_EXAMPLE_DIR}/exp/parallel.cpp)
ink_add_example(part_policy    ${INK_EXAMPLE_DIR}/exp/part_policy.cpp)
ink_add_example(acc            ${INK_EXAMPLE_DIR}/exp/accuracy.cpp)
ink_add_example(single-iter-ot ${INK_EXAMPLE_DIR}/exp/single-iter-ot.cpp)
ink_add_example(single-iter-pathgen ${INK_EXAMPLE_DIR}/exp/single-iter-pathgen.cpp)

# cpathgen
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INK_EXAMPLE_DIR}/cpathgen)
ink_add_example(toy             ${INK_EXAMPLE_DIR}/cpathgen/toy.cpp)
ink_add_example(gen-golden      ${INK_EXAMPLE_DIR}/cpathgen/gen-golden.cpp)
ink_add_example(gen-golden-trace ${INK_EXAMPLE_DIR}/cpathgen/gen-golden-trace.cpp)
ink_add_example(big-table       ${INK_EXAMPLE_DIR}/cpathgen/big-table.cpp)
ink_add_example(exactness-table ${INK_EXAMPLE_DIR}/cpathgen/exactness-table.cpp)
ink_add_example(exactness-batch ${INK_EXAMPLE_DIR}/cpathgen/exactness-batch.cpp)
ink_add_example(kernel-sweep    ${INK_EXAMPLE_DIR}/cpathgen/kernel-sweep.cpp)
ink_add_example(cpathgen-only-sweep ${INK_EXAMPLE_DIR}/cpathgen/cpathgen-only-sweep.cpp)
ink_add_example(dump-paths      ${INK_EXAMPLE_DIR}/cpathgen/dump-paths.cpp)
ink_add_example(convert-gpathgen-benchmark-to-edge-insertions
  ${INK_EXAMPLE_DIR}/cpathgen/convert-gpathgen-benchmark-to-edge-insertions.cpp)
ink_add_example(speedup-acc-vs-num-threads
  ${INK_EXAMPLE_DIR}/cpathgen/speedup-acc-vs-num-threads.cpp)
ink_add_example(runtime-vs-path-cnt
  ${INK_EXAMPLE_DIR}/cpathgen/runtime-vs-path-cnt.cpp)
ink_add_example(runtime-steps-vs-delta
  ${INK_EXAMPLE_DIR}/cpathgen/runtime-steps-vs-delta.cpp)
ink_add_example(err-vs-path-cnt
  ${INK_EXAMPLE_DIR}/cpathgen/err-vs-path-cnt.cpp)

find_package(Threads REQUIRED)
foreach(eg IN LISTS INK_EXAMPLES)
  target_include_directories(${eg}
    PUBLIC
    ${INK_3RD_PARTY_DIR}/OpenTimer
    ${PROJECT_SOURCE_DIR})

  target_link_libraries(${eg}
    PRIVATE Threads::Threads
    PUBLIC OpenTimer ink)
endforeach()
