cmake_minimum_required(VERSION 3.14)

# Explicitly set compiler for MSVC to avoid environment variable conflicts
if(MSVC)
    # This path is typically where cl.exe resides for Visual Studio 2019
    # Adjust as necessary if the exact path differs on the user's system
    find_program(MSVC_CL_EXE NAMES cl.exe
                 PATHS "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.29.30037/bin/Hostx64/x64"
                 NO_DEFAULT_PATH
                 REQUIRED)
    set(CMAKE_C_COMPILER "${MSVC_CL_EXE}")
    set(CMAKE_CXX_COMPILER "${MSVC_CL_EXE}")

    # Set Windows SDK Version explicitly
    set(CMAKE_SYSTEM_VERSION 10.0) # This should correspond to the desired Windows SDK

    # Limit CMake's search paths to avoid conflicts with other toolchains
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
endif()

project(GardaProject)

# Ensure proper C runtime library linking for MSVC
if(MSVC)
    # Clear system include paths to prevent unintended conflicts
    set(CMAKE_SYSTEM_INCLUDE_PATH "")
    # Prevent accidental cross-compilation
    set(CMAKE_CROSSCOMPILING_EMULATOR "")

    # Explicitly set the C runtime library to avoid LNK2038 "RuntimeLibrary mismatch" errors.
    # MDd for Debug, MD for Release (Dynamic linking to C runtime)
    foreach(config IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
        if(config STREQUAL "Debug")
            string(APPEND CMAKE_CXX_FLAGS_${config} " /MDd")
            string(APPEND CMAKE_C_FLAGS_${config} " /MDd")
        else()
            string(APPEND CMAKE_CXX_FLAGS_${config} " /MD")
            string(APPEND CMAKE_C_FLAGS_${config} " /MD")
        endif()
    endforeach()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --------------------------------------------------------------------
# Dependencies
# --------------------------------------------------------------------
include(FetchContent)

# --- cpp-httplib (via submodule) ---
add_subdirectory(extern/cpp-httplib)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)
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


# --- CLI Client Executable ---
add_executable(calc_client client/calc_cli.cpp)
target_link_libraries(calc_client PRIVATE 
    httplib::httplib
    nlohmann_json::nlohmann_json
)

# --- Mock Server Executable for Client Tests ---
add_executable(mock_server test/mock_server.cpp)
target_link_libraries(mock_server PRIVATE
    httplib::httplib
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

# --- Client Integration Tests ---
add_executable(client_tests
    test/client_test.cpp
)
target_link_libraries(client_tests PRIVATE
    gtest_main
    httplib::httplib
    nlohmann_json::nlohmann_json
)
gtest_discover_tests(client_tests)
