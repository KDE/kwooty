cmake_minimum_required(VERSION 2.6)
project (Kwooty)

set(KDE_MIN_VERSION "4.6.0")
find_package(KDE4 REQUIRED)
find_package(KDE4Workspace REQUIRED)
include(KDE4Defaults)

macro_ensure_version("4.3.80" ${KDE_VERSION} KSTATUSNOTIFIERITEM_FOUND)
macro_bool_to_01(KSTATUSNOTIFIERITEM_FOUND HAVE_KSTATUSNOTIFIERITEM)

macro_ensure_version("4.5.82" ${KDE_VERSION} NEW_SOLID_POWERMANAGEMENT_FOUND)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src)
