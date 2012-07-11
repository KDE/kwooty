include_directories(${KWOOTY_INCLUDES})

# plugin target :
set(kwooty_categoriesplugin_PART_SRCS categoriesplugin.cpp categories.cpp categoriesfilehandler.cpp mimedata.cpp categoriesmodel.cpp utilitycategories.cpp)
kde4_add_kcfg_files(kwooty_categoriesplugin_PART_SRCS kwooty_categoriessettings.kcfgc)
kde4_add_plugin(kwooty_categoriesplugin ${kwooty_categoriesplugin_PART_SRCS})
target_link_libraries(kwooty_categoriesplugin ${KDE4_KIO_LIBS} kwootycore ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})


# plugin config target :
set(kwooty_categoriessettings_PART_SRCS preferencescategories.cpp categoriesfilehandler.cpp mimedata.cpp categoriesmodel.cpp utilitycategories.cpp)
kde4_add_ui_files(kwooty_categoriessettings_PART_SRCS preferencescategories.ui)
kde4_add_kcfg_files(kwooty_categoriessettings_PART_SRCS kwooty_categoriessettings.kcfgc)
kde4_add_plugin(kwooty_categoriessettings ${kwooty_categoriessettings_PART_SRCS})
target_link_libraries(kwooty_categoriessettings ${KDE4_KIO_LIBS} kwootycore ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS})


install(TARGETS kwooty_categoriessettings kwooty_categoriesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kwooty_categoriessettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kwooty_categoriessettings.desktop kwooty_categoriesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})



  
