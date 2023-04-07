add_executable(InkTimer main.cpp)

# options/flags pass to the compiler
target_compile_features(inktimer INTERFACE cxx_std_20)
target_compile_options(inktimer INTERFACE -Wall -Wextra -Wfatal-errors)


# include and static libary linking
# target_link_libraries(qsat LINK_PUBLIC ${PROJECT_SOURCE_DIR}/libmetis/libmetis.a
#                           LINK_PUBLIC ${PROJECT_SOURCE_DIR}/libmetis/libGKlib.a
#                           LINK_PUBLIC ${PROJECT_SOURCE_DIR}/libbliss/libbliss_static.a)

target_link_libraries(InkTimer
  LINK_PUBLIC inktimer 
)

target_include_directories(InkTimer PRIVATE ${PROJECT_SOURCE_DIR})

# create a variable for this executable
set(INKTIMER_EXEC ${PROJECT_SOURCE_DIR}/build/main/InkTimer)

