include_directories(  ${KDE4WORKSPACE_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

set(KWOOTY_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/plugins/
)


add_subdirectory(icons)
#KF5 reactivate it
#add_subdirectory(plugins)

set(Kwooty_SRCS 
  main.cpp
  uniqueapp.cpp
)

set(Kwooty_lib_SRCS
  kwooty_debug.cpp 
  mainwindow.cpp
  core.cpp
  nzbfilehandler.cpp
  clientmanagerconn.cpp
  nntpsocket.cpp
  nntpclient.cpp
  itemdelegate.cpp
  itemdownloadupdater.cpp
  itempostdownloadupdater.cpp
  itemparentupdater.cpp
  itemabstractupdater.cpp
  itemchildrenmanager.cpp
  standarditemmodel.cpp
  standarditemmodelquery.cpp
  segmentmanager.cpp
  segmentbuffer.cpp
  segmentsdecoderthread.cpp
  segmentdecoderbase.cpp
  segmentdecoderyenc.cpp
  segmentdecoderuuenc.cpp
  repairdecompressthread.cpp
  repair.cpp
  extractbase.cpp
  extractrar.cpp
  extractzip.cpp
  extractsplit.cpp
  datarestorer.cpp
  shutdown/shutdownmanager.cpp
  shutdown/sessionbase.cpp
  shutdown/sessionkde.cpp
  shutdown/sessiongnome2.cpp
  shutdown/sessiongnome3.cpp
  fileoperations.cpp
  statsinfobuilder.cpp
  notificationmanager.cpp
  servermanager.cpp
  servergroup.cpp
  serverspeedmanager.cpp
  sidebar.cpp
  actions/actionsmanager.cpp
  actions/actionbuttonsmanager.cpp
  actions/actionfilemanagerbase.cpp
  actions/actionmergemanager.cpp
  actions/actionrenamemanager.cpp
  actions/actionfiledeletemanager.cpp
  utilities/utility.cpp
  utilities/utilityiconpainting.cpp
  utilities/utilityserverstatus.cpp
  observers/clientsobserver.cpp
  observers/clientsperserverobserver.cpp
  observers/clientsobserverbase.cpp
  observers/queuefileobserver.cpp
  preferences/preferencesserver.cpp
  preferences/preferencesgeneral.cpp
  preferences/preferencesprograms.cpp
  preferences/preferencesdisplay.cpp
  preferences/preferencesshutdown.cpp
  preferences/preferencesplugins.cpp
  preferences/kconfiggrouphandler.cpp
  plugins/plugin.cpp
  plugins/pluginmanager.cpp
  data/segmentdata.cpp
  data/segmentinfodata.cpp
  data/nzbfiledata.cpp
  data/itemstatusdata.cpp
  data/globalfiledata.cpp 
  data/nzbcollectiondata.cpp
  data/jobnotifydata.cpp
  data/serverdata.cpp
  data/postdownloadinfodata.cpp
  widgets/centralwidget.cpp
  widgets/mytreeview.cpp
  widgets/mystatusbar.cpp
  widgets/icontextwidget.cpp
  widgets/iconcapacitywidget.cpp
  widgets/serverpreferenceswidget.cpp
  widgets/servertabwidget.cpp
  widgets/sidebarwidget.cpp
  widgets/serverstatuswidget.cpp
  widgets/textpushbuttonwidget.cpp
  widgets/statusbarwidgetbase.cpp
  jobs/concatsplitfilesjob.cpp
)

if(KSTATUSNOTIFIERITEM_FOUND)
    set(Kwooty_lib_SRCS ${Kwooty_lib_SRCS} systray.cpp)
else(KSTATUSNOTIFIERITEM_FOUND)
    set(Kwooty_lib_SRCS ${Kwooty_lib_SRCS} systraylegacy.cpp)
endif(KSTATUSNOTIFIERITEM_FOUND)



ki18n_wrap_ui(  Kwooty_lib_SRCS
		    preferences/preferencesserver.ui
		    preferences/preferencesgeneral.ui
		    preferences/preferencesprograms.ui
		    preferences/preferencesdisplay.ui
		    preferences/preferencesshutdown.ui
		    preferences/serversettings.ui
		 )
kconfig_add_kcfg_files(Kwooty_lib_SRCS preferences/kwootysettings.kcfgc)


add_library(kwootycore SHARED ${Kwooty_lib_SRCS})
generate_export_header(kwootycore BASE_NAME kwooty)

target_link_libraries(kwootycore
		      KF5::KDELibs4Support
		      KF5::KIOCore
                      KF5::Wallet
                      KF5::KCMUtils
		      ${KDE4WORKSPACE_KWORKSPACE_LIBS}
  )

if (NOT NEW_SOLID_POWERMANAGEMENT_FOUND) 
  target_link_libraries(kwootycore
			${KDE4WORKSPACE_SOLIDCONTROL_LIBS}
  )
endif(NOT NEW_SOLID_POWERMANAGEMENT_FOUND)



set_target_properties(kwootycore PROPERTIES VERSION 1.1.0 SOVERSION 0)
install(TARGETS kwootycore ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

add_executable(kwooty ${Kwooty_SRCS})
target_link_libraries(kwooty  kwootycore )


install(TARGETS kwooty DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES kwootyui.rc DESTINATION ${KDE_INSTALL_DATADIR}/kwooty)
install(FILES kwooty.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFY5RCDIR})
install(FILES preferences/kwootysettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES org.kde.kwooty.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES kwootyplugin.desktop DESTINATION  ${KDE_INSTALL_KSERVICETYPES5DIR} )
