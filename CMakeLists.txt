cmake_minimum_required(VERSION 3.14)

project(GardaHttpServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(extern/cpp-httplib)

add_executable(echo_server main.cpp)

target_link_libraries(echo_server PRIVATE httplib::httplib)