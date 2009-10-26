cmake_minimum_required(VERSION 2.6)
project (Kwooty)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/src/uudeviewCMake")

find_package(KDE4 REQUIRED)
find_package(UUDEVIEW REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src)