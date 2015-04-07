include_directories(${KWOOTY_INCLUDES})

# plugin target :
set(kwooty_watchfolderplugin_PART_SRCS watchfolderplugin.cpp watchfolder.cpp)
kconfig_add_kcfg_files(kwooty_watchfolderplugin_PART_SRCS kwooty_watchfoldersettings.kcfgc)
add_library(kwooty_watchfolderplugin MODULE ${kwooty_watchfolderplugin_PART_SRCS})
target_link_libraries(kwooty_watchfolderplugin KF5::KIOCore kwootycore  )


# plugin config target :
set(kwooty_watchfoldersettings_PART_SRCS preferenceswatchfolder.cpp)
ki18n_wrap_ui(kwooty_watchfoldersettings_PART_SRCS preferenceswatchfolder.ui)
kconfig_add_kcfg_files(kwooty_watchfoldersettings_PART_SRCS kwooty_watchfoldersettings.kcfgc)
add_library(kwooty_watchfoldersettings MODULE ${kwooty_watchfoldersettings_PART_SRCS})
target_link_libraries(kwooty_watchfoldersettings KF5::KIOCore kwootycore  )


install(TARGETS kwooty_watchfoldersettings kwooty_watchfolderplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES kwooty_watchfoldersettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kwooty_watchfoldersettings.desktop kwooty_watchfolderplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})



  