include_directories(${KWOOTY_INCLUDES})

# plugin target :
set(kwooty_autoretryplugin_PART_SRCS autoretryplugin.cpp autoretry.cpp)
kde4_add_kcfg_files(kwooty_autoretryplugin_PART_SRCS kwooty_autoretrysettings.kcfgc)
kde4_add_plugin(kwooty_autoretryplugin ${kwooty_autoretryplugin_PART_SRCS})
target_link_libraries(kwooty_autoretryplugin ${KDE4_KIO_LIBS} kwootycore ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})


# plugin config target :
set(kwooty_autoretrysettings_PART_SRCS preferencesautoretry.cpp)
kde4_add_ui_files(kwooty_autoretrysettings_PART_SRCS preferencesautoretry.ui)
kde4_add_kcfg_files(kwooty_autoretrysettings_PART_SRCS kwooty_autoretrysettings.kcfgc)
kde4_add_plugin(kwooty_autoretrysettings ${kwooty_autoretrysettings_PART_SRCS})
target_link_libraries(kwooty_autoretrysettings ${KDE4_KIO_LIBS} kwootycore ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})


install(TARGETS kwooty_autoretrysettings kwooty_autoretryplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kwooty_autoretrysettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kwooty_autoretrysettings.desktop kwooty_autoretryplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})



  