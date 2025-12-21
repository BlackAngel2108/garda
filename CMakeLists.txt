cmake_minimum_required(VERSION 3.14)

project(GardaProject) # More generic project name

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------

# --- cpp-httplib (via submodule) ---
add_subdirectory(extern/cpp-httplib)

# --- Google Test (via FetchContent) ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip # Release 1.10.0 to avoid potential master branch issues
)
# For Windows: GTest requires this to be set to avoid mixing static/dynamic CRT
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# --------------------------------------------------------------------
# Project Libraries
# --------------------------------------------------------------------

# --- Calculator Library ---
add_library(calculator STATIC
    calculator/src/calculator.cpp
)
target_include_directories(calculator PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/calculator/include
)


# --------------------------------------------------------------------
# Executables
# --------------------------------------------------------------------

# --- Echo Server Executable ---
add_executable(echo_server main.cpp)
target_link_libraries(echo_server PRIVATE httplib::httplib)


# --------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------
enable_testing()

add_executable(calculator_tests 
    calculator/test/calculator_test.cpp
)
target_link_libraries(calculator_tests PRIVATE 
    calculator
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(calculator_tests)
