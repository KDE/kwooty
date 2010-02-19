include_directories( ${KDE4_INCLUDES} ${KWORKSPACE_INCLUDE_DIR}  ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} )
add_subdirectory(icons)

set(Kwooty_SRCS 
  main.cpp
  uniqueapp.cpp
  mainwindow.cpp
  centralwidget.cpp
  nzbfilehandler.cpp
  utility.cpp
  clientmanagerconn.cpp
  nntpclient.cpp
  itemdelegate.cpp
  itemdownloadupdater.cpp
  itempostdownloadupdater.cpp
  itemparentupdater.cpp
  itemabstractupdater.cpp
  itemchildrenmanager.cpp
  standarditemmodel.cpp
  segmentmanager.cpp
  mystatusbar.cpp
  mytreeview.cpp
  segmentsdecoderthread.cpp
  segmentdecoderbase.cpp
  segmentdecoderyenc.cpp
  segmentdecoderuuenc.cpp
  repairdecompressthread.cpp
  repair.cpp
  extract.cpp
  datarestorer.cpp
  shutdownmanager.cpp
  preferences/preferencesserver.cpp
  preferences/preferencesgeneral.cpp
  preferences/preferencesprograms.cpp
  preferences/preferencesdisplay.cpp
  preferences/preferencesshutdown.cpp
  data/segmentdata.cpp
  data/nzbfiledata.cpp
  data/itemstatusdata.cpp
  data/globalfiledata.cpp 

)

kde4_add_ui_files(  Kwooty_SRCS
		    preferences/preferencesserver.ui
		    preferences/preferencesgeneral.ui
		    preferences/preferencesprograms.ui
		    preferences/preferencesdisplay.ui
		    preferences/preferencesshutdown.ui
		 )
kde4_add_kcfg_files(Kwooty_SRCS preferences/settings.kcfgc)

kde4_add_executable(kwooty ${Kwooty_SRCS})

target_link_libraries(kwooty ${KDE4_KDEUI_LIBS}
			     ${KDE4_KDECORE_LIBS}
			     ${KDE4_KIO_LIBS}
			     ${KDE4WORKSPACE_KWORKSPACE_LIBS}
			     ${KDE4WORKSPACE_SOLIDCONTROL_LIBS})


install(TARGETS kwooty DESTINATION ${BIN_INSTALL_DIR})
install(FILES kwootyui.rc DESTINATION  ${DATA_INSTALL_DIR}/kwooty)
install(FILES preferences/settings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kwooty.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
