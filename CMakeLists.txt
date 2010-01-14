cmake_minimum_required(VERSION 2.6)
project (Kwooty)

find_package(KDE4 REQUIRED)
find_package(KDE4Workspace REQUIRED)
include(KDE4Defaults)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src)

