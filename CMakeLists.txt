cmake_minimum_required(VERSION 3.14)

project(GardaProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------
include(FetchContent)

# --- cpp-httplib (via submodule) ---
add_subdirectory(extern/cpp-httplib)

# --- nlohmann/json (via FetchContent) ---
# FetchContent_Declare(
#   nlohmann_json
#   GIT_REPOSITORY https://github.com/nlohmann/json.git
#   GIT_TAG v3.10.5 # Pinned to a specific version for reproducibility
# )
# FetchContent_MakeAvailable(nlohmann_json)
add_subdirectory(extern/nlohmann_json)

# --- Google Test (via FetchContent) ---
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip # Release 1.10.0
# )
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)
add_subdirectory(extern/googletest)
include(GoogleTest)


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
# Main Executable
# --------------------------------------------------------------------

# --- HTTP Server Executable ---
add_executable(http_server main.cpp)
target_link_libraries(http_server PRIVATE 
    httplib::httplib
    calculator
    nlohmann_json::nlohmann_json
)


# --------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------
enable_testing()

# --- Calculator Unit Tests ---
add_executable(calculator_tests 
    calculator/test/calculator_test.cpp
)
target_link_libraries(calculator_tests PRIVATE 
    calculator
    gtest_main
)
gtest_discover_tests(calculator_tests)

# --- Server Integration Tests ---
add_executable(server_tests
    test/server_test.cpp
)
target_link_libraries(server_tests PRIVATE
    httplib::httplib
    calculator
    nlohmann_json::nlohmann_json
    gtest_main
)
gtest_discover_tests(server_tests)