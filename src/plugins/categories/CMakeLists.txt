include_directories(${KWOOTY_INCLUDES})

set(Kwooty_categoriesglobal_SRCS 
 categoriesfilehandler.cpp
 mimedata.cpp
 categoriesmodel.cpp
 utilitycategories.cpp
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


install(TARGETS kwooty_categoriessettings kwooty_categoriesplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kwooty_categoriessettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kwooty_categoriessettings.desktop kwooty_categoriesplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})



  
