project (Kwooty)
cmake_minimum_required(VERSION 2.8.12)

# we need some parts of the ECM CMake helpers
find_package(ECM 5.9.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${kdepim_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMMarkNonGuiExecutable)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMAddAppIcon)


set(KF5_VERSION "5.9.0")
find_package(KF5Wallet ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KDELibs4Support ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5NotifyConfig ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Config ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5Service ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DBusAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KCMUtils ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5DocTools REQUIRED)


add_subdirectory(src)

#set(KDE_MIN_VERSION "4.6.0")
#find_package(KDE4 REQUIRED)
#find_package(KDE4Workspace REQUIRED)
#include(KDE4Defaults)

#macro_ensure_version("4.3.80" ${KDE_VERSION} KSTATUSNOTIFIERITEM_FOUND)
#macro_bool_to_01(KSTATUSNOTIFIERITEM_FOUND HAVE_KSTATUSNOTIFIERITEM)

#macro_ensure_version("4.5.82" ${KDE_VERSION} NEW_SOLID_POWERMANAGEMENT_FOUND)

#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#add_subdirectory(src)
