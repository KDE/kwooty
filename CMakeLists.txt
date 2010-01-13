cmake_minimum_required(VERSION 2.6)
project (Kwooty)

find_package(KDE4 REQUIRED)

include(FindKDE4Workspace)
if (NOT KDE4Workspace_CONFIG)
  message(FATAL_ERROR "ERROR: \"kdebase workspace devel\" package is required. e.g: install kdebase-workspace-dev or kdebase4-workspace-devel package.")
endif (NOT KDE4Workspace_CONFIG)


include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src)

