project(kscd)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kscd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kscd.h )

include_directories(
		${CMAKE_SOURCE_DIR}/libkcddb
		${CMAKE_BINARY_DIR}/libkcddb
		${CMAKE_SOURCE_DIR}/libkcompactdisc
		${CMAKE_BINARY_DIR}/libkcompactdisc
		${CMAKE_BINARY_DIR}/kscd
		)

########### next target ###############


########### next target ###############

set(kscd_SRCS kscd.cpp ledlamp.cpp docking.cpp bwlednum.cpp  configWidget.cpp cddbdlg.cpp)

kde4_add_ui_files(kscd_SRCS panel.ui configWidgetUI.ui )

kde4_add_kcfg_files(kscd_SRCS prefs.kcfgc )

kde4_automoc(${kscd_SRCS})

kde4_add_executable(kscd ${kscd_SRCS})

target_link_libraries(kscd  ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} kcddb kcompactdisc)

install(TARGETS kscd  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES kscd.desktop  DESTINATION  ${XDG_APPS_DIR} )
install( FILES kscd.profile.xml  DESTINATION  ${DATA_INSTALL_DIR}/profiles )
install( FILES kscd.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES audiocd_play.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqueror/servicemenus )
install( FILES xmcd.desktop  DESTINATION  ${MIME_INSTALL_DIR}/text )
install(PROGRAMS workman2cddb.pl DESTINATION ${BIN_INSTALL_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}   )

