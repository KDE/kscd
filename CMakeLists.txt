project(kscd)

include_directories(
		${CMAKE_SOURCE_DIR}/libkcddb
		${CMAKE_BINARY_DIR}/libkcddb
		${CMAKE_SOURCE_DIR}/libkcompactdisc
		${CMAKE_BINARY_DIR}/libkcompactdisc
		${CMAKE_SOURCE_DIR}/kscd
		${CMAKE_BINARY_DIR}/kscd
		)

add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############


########### next target ###############

set(kscd_SRCS kscd.cpp docking.cpp bwlednum.cpp configWidget.cpp cddbdlg.cpp cdcombobox.cpp)

kde4_add_ui_files(kscd_SRCS panel.ui configWidgetUI.ui)

kde4_add_kcfg_files(kscd_SRCS prefs.kcfgc)


kde4_add_executable(kscd ${kscd_SRCS})

target_link_libraries(kscd ${KDE4_KUTILS_LIBS} kcddb kcompactdisc)

install(TARGETS kscd DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install(FILES kscd.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES kscd.profile.xml  DESTINATION  ${DATA_INSTALL_DIR}/profiles)
install(FILES kscd.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES audiocd_play.desktop  DESTINATION  ${DATA_INSTALL_DIR}/konqueror/servicemenus)
install(PROGRAMS workman2cddb.pl DESTINATION ${BIN_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})

