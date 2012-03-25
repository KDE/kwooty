include_directories(${KWOOTY_INCLUDES})

# plugin target :
set(kwooty_watchfolderplugin_PART_SRCS watchfolderplugin.cpp watchfolder.cpp)
kde4_add_kcfg_files(kwooty_watchfolderplugin_PART_SRCS kwooty_watchfoldersettings.kcfgc)
kde4_add_plugin(kwooty_watchfolderplugin ${kwooty_watchfolderplugin_PART_SRCS})
target_link_libraries(kwooty_watchfolderplugin ${KDE4_KIO_LIBS} kwootycore ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})


# plugin config target :
set(kwooty_watchfoldersettings_PART_SRCS preferenceswatchfolder.cpp)
kde4_add_ui_files(kwooty_watchfoldersettings_PART_SRCS preferenceswatchfolder.ui)
kde4_add_kcfg_files(kwooty_watchfoldersettings_PART_SRCS kwooty_watchfoldersettings.kcfgc)
kde4_add_plugin(kwooty_watchfoldersettings ${kwooty_watchfoldersettings_PART_SRCS})
target_link_libraries(kwooty_watchfoldersettings ${KDE4_KIO_LIBS} kwootycore ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})


install(TARGETS kwooty_watchfoldersettings kwooty_watchfolderplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kwooty_watchfoldersettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kwooty_watchfoldersettings.desktop kwooty_watchfolderplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})



  