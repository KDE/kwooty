include_directories(${KWOOTY_INCLUDES})

# plugin target :
set(kwooty_schedulerplugin_PART_SRCS schedulerplugin.cpp scheduler.cpp schedulerfilehandler.cpp)
kconfig_add_kcfg_files(kwooty_schedulerplugin_PART_SRCS kwooty_schedulersettings.kcfgc)
add_library(kwooty_schedulerplugin MODULE ${kwooty_schedulerplugin_PART_SRCS})
target_link_libraries(kwooty_schedulerplugin KF5::KIOCore kwootycore  )


# plugin config target :
set(kwooty_schedulersettings_PART_SRCS preferencesscheduler.cpp schedulertableitemdelegate.cpp schedulerfilehandler.cpp)
ki18n_wrap_ui(kwooty_schedulersettings_PART_SRCS preferencesscheduler.ui)
kconfig_add_kcfg_files(kwooty_schedulersettings_PART_SRCS kwooty_schedulersettings.kcfgc)
add_library(kwooty_schedulersettings MODULE ${kwooty_schedulersettings_PART_SRCS})
target_link_libraries(kwooty_schedulersettings KF5::KIOCore kwootycore  )


install(TARGETS kwooty_schedulersettings kwooty_schedulerplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kwooty_schedulersettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kwooty_schedulersettings.desktop kwooty_schedulerplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})



  