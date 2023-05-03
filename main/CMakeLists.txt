add_executable(Ink main.cpp)

# options/flags pass to the compiler
target_compile_features(ink INTERFACE cxx_std_20)
target_compile_options(ink INTERFACE -Wall -Wextra -Wfatal-errors)

target_link_libraries(Ink
  LINK_PUBLIC ink
)

target_include_directories(Ink PRIVATE ${PROJECT_SOURCE_DIR})

# create a variable for this executable
set(INKTIMER_EXEC ${PROJECT_SOURCE_DIR}/build/main/Ink)

