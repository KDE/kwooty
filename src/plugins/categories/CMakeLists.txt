include_directories(${KWOOTY_INCLUDES})

set(Kwooty_categoriesglobal_SRCS 
 categoriesfilehandler.cpp
 mimedata.cpp
 categoriesmodel.cpp
 utilitycategories.cpp
 kwooty_categories_plugins_debug.cpp
)

# plugin target :
set(kwooty_categoriesplugin_PART_SRCS categoriesplugin.cpp categories.cpp categoriesmanual.cpp ${Kwooty_categoriesglobal_SRCS})
kconfig_add_kcfg_files(kwooty_categoriesplugin_PART_SRCS kwooty_categoriessettings.kcfgc)
add_library(kwooty_categoriesplugin MODULE ${kwooty_categoriesplugin_PART_SRCS})
target_link_libraries(kwooty_categoriesplugin KF5::KIOCore kwootycore  )


# plugin config target :
set(kwooty_categoriessettings_PART_SRCS preferencescategories.cpp ${Kwooty_categoriesglobal_SRCS})
ki18n_wrap_ui(kwooty_categoriessettings_PART_SRCS preferencescategories.ui)
kconfig_add_kcfg_files(kwooty_categoriessettings_PART_SRCS kwooty_categoriessettings.kcfgc)
add_library(kwooty_categoriessettings MODULE ${kwooty_categoriessettings_PART_SRCS})
target_link_libraries(kwooty_categoriessettings KF5::KIOCore kwootycore  )


install(TARGETS kwooty_categoriessettings kwooty_categoriesplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES kwooty_categoriessettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kwooty_categoriessettings.desktop kwooty_categoriesplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})



  
