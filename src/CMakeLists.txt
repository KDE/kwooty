include_directories( ${KDE4_INCLUDES} ${KDE4WORKSPACE_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )

set(KWOOTY_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/plugins/
)


add_subdirectory(icons)
add_subdirectory(plugins)

set(Kwooty_SRCS 
  main.cpp
  uniqueapp.cpp
)

set(Kwooty_lib_SRCS 
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



kde4_add_ui_files(  Kwooty_lib_SRCS
		    preferences/preferencesserver.ui
		    preferences/preferencesgeneral.ui
		    preferences/preferencesprograms.ui
		    preferences/preferencesdisplay.ui
		    preferences/preferencesshutdown.ui
		    preferences/serversettings.ui
		 )
kde4_add_kcfg_files(Kwooty_lib_SRCS preferences/kwootysettings.kcfgc)


kde4_add_library(kwootycore SHARED ${Kwooty_lib_SRCS})

target_link_libraries(kwootycore
		      ${KDE4_KDECORE_LIBS}
		      ${KDE4_KIO_LIBS}
		      ${KDE4_KUTILS_LIBS}
		      ${KDE4_SOLID_LIBS}
		      ${KDE4WORKSPACE_KWORKSPACE_LIBS}
  )

if (NOT NEW_SOLID_POWERMANAGEMENT_FOUND) 
  target_link_libraries(kwootycore
			${KDE4WORKSPACE_SOLIDCONTROL_LIBS}
  )
endif(NOT NEW_SOLID_POWERMANAGEMENT_FOUND)



set_target_properties(kwootycore PROPERTIES VERSION 1.1.0 SOVERSION 0)
install(TARGETS kwootycore ${INSTALL_TARGETS_DEFAULT_ARGS} )

kde4_add_executable(kwooty ${Kwooty_SRCS})
target_link_libraries(kwooty ${KDE4_KDEUI_LIBS} kwootycore )


install(TARGETS kwooty DESTINATION ${BIN_INSTALL_DIR})
install(FILES kwootyui.rc DESTINATION ${DATA_INSTALL_DIR}/kwooty)
install(FILES kwooty.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kwooty)
install(FILES preferences/kwootysettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kwooty.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kwootyplugin.desktop DESTINATION  ${SERVICETYPES_INSTALL_DIR} )
