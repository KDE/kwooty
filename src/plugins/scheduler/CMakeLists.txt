include_directories(${KWOOTY_INCLUDES})

# plugin target :
set(kwooty_schedulerplugin_PART_SRCS schedulerplugin.cpp scheduler.cpp)
kde4_add_kcfg_files(kwooty_schedulerplugin_PART_SRCS kwooty_schedulersettings.kcfgc)
kde4_add_plugin(kwooty_schedulerplugin ${kwooty_schedulerplugin_PART_SRCS})
target_link_libraries(kwooty_schedulerplugin ${KDE4_KIO_LIBS} kwootycore ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})


# plugin config target :
set(kwooty_schedulersettings_PART_SRCS preferencesscheduler.cpp schedulertableitemdelegate.cpp)
kde4_add_ui_files(kwooty_schedulersettings_PART_SRCS preferencesscheduler.ui)
kde4_add_kcfg_files(kwooty_schedulersettings_PART_SRCS kwooty_schedulersettings.kcfgc)
kde4_add_plugin(kwooty_schedulersettings ${kwooty_schedulersettings_PART_SRCS})
target_link_libraries(kwooty_schedulersettings ${KDE4_KIO_LIBS} kwootycore ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})


install(TARGETS kwooty_schedulersettings kwooty_schedulerplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kwooty_schedulersettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kwooty_schedulersettings.desktop kwooty_schedulerplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})



  