cmake_minimum_required(VERSION 2.6)
project (Kwooty)

find_package(KDE4 REQUIRED)
find_package(KDE4Workspace REQUIRED)
include(KDE4Defaults)

macro_ensure_version("4.3.80" ${KDE_VERSION} KSTATUSNOTIFIERITEM_FOUND)
macro_bool_to_01(KSTATUSNOTIFIERITEM_FOUND HAVE_KSTATUSNOTIFIERITEM)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src)

