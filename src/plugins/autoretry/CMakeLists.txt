include_directories(${KWOOTY_INCLUDES})

# plugin target :
set(kwooty_autoretryplugin_PART_SRCS autoretryplugin.cpp autoretry.cpp)
kconfig_add_kcfg_files(kwooty_autoretryplugin_PART_SRCS kwooty_autoretrysettings.kcfgc)
add_library(kwooty_autoretryplugin MODULE ${kwooty_autoretryplugin_PART_SRCS})
target_link_libraries(kwooty_autoretryplugin KF5::KIOCore kwootycore  )


# plugin config target :
set(kwooty_autoretrysettings_PART_SRCS preferencesautoretry.cpp)
ki18n_wrap_ui(kwooty_autoretrysettings_PART_SRCS preferencesautoretry.ui)
kconfig_add_kcfg_files(kwooty_autoretrysettings_PART_SRCS kwooty_autoretrysettings.kcfgc)
add_library(kwooty_autoretrysettings MODULE ${kwooty_autoretrysettings_PART_SRCS})
target_link_libraries(kwooty_autoretrysettings KF5::KIOCore kwootycore  )


install(TARGETS kwooty_autoretrysettings kwooty_autoretryplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES kwooty_autoretrysettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kwooty_autoretrysettings.desktop kwooty_autoretryplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})



  