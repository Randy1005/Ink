enable_testing()

include(${INKTIMER_3RD_PARTY_DIR}/doctest/doctest.cmake)

list(APPEND INKTIMER_UNITTESTS

)

foreach(unittest IN LISTS INKTIMER_UNITTESTS)
  add_executable(${unittest} ${unittest}.cpp)
	add_test(NAME ${unittest} COMMAND ${unittest})	
	set_property(TEST ${unittest} PROPERTY LABELS "unit")
  target_include_directories(${unittest} 
    PRIVATE ${PROJECT_SOURCE_DIR} ${INKTIMER_3RD_PARTY_DIR}/doctest)
  target_link_libraries(${unittest} 
    LINK_PUBLIC qsat stdc++fs)
  
  doctest_discover_tests(${unittest})
endforeach()

