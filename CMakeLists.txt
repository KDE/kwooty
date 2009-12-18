cmake_minimum_required(VERSION 2.6)
project (Kwooty)

find_package(KDE4 REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src)