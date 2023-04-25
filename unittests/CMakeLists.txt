enable_testing()
include(${INK_3RD_PARTY_DIR}/doctest/doctest.cmake)
list(APPEND INK_UNITTESTS
  basics
  sample_graph
  c17_graph
  c3_slack_graph
  simp_s27_graph
  inc_sample_graph
)

foreach(unittest IN LISTS INK_UNITTESTS)
  add_executable(${unittest} ${unittest}.cpp)
	add_test(NAME ${unittest} COMMAND ${unittest})	
	set_property(TEST ${unittest} PROPERTY LABELS "ink-unit")
  target_include_directories(${unittest} 
    PRIVATE ${PROJECT_SOURCE_DIR} ${INK_3RD_PARTY_DIR}/doctest)
  
  target_link_libraries(${unittest} 
    LINK_PUBLIC ink stdc++fs)
  
  doctest_discover_tests(${unittest})
endforeach()

