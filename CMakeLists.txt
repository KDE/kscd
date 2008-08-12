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
add_subdirectory(tests)
########### next target ###############

set(kscd_SRCS 
	kscd.cpp 
	hwcontroler.cpp 
	audiocd.cpp 
# 	bwlednum.cpp 
# 	configWidget.cpp 
# 	cddbdlg.cpp 
	mbmanager.cpp
	ihm/kscdwindow.cpp 
	ihm/kscdwidget.cpp 
	ihm/ejectbutton.cpp 
	ihm/stopbutton.cpp 
	ihm/playbutton.cpp 
	ihm/nextbutton.cpp 
	ihm/previousbutton.cpp 
	ihm/mutebutton.cpp
	ihm/randombutton.cpp
	ihm/loopbutton.cpp
	ihm/tracklistbutton.cpp
	ihm/volumebutton.cpp
	ihm/panel.cpp
	ihm/titlePopUp.cpp
	ihm/tracklistdlg.cpp
	ihm/background.cpp
	ihm/closebutton.cpp
	ihm/minimizebutton.cpp
	ihm/finderSkin.cpp
	ihm/seekslider.cpp
	ihm/seekcursor.cpp
	ihm/seekbar.cpp
)

file(GLOB resource_svg "ihm/skin/*.svg")

kde4_add_ui_files(kscd_SRCS ihm/trackListDlgUI.ui ihm/finderSkinUI.ui generalSettings.ui interfaceSettings.ui)

kde4_add_kcfg_files(kscd_SRCS prefs.kcfgc)

#kde_kcfg_DATA(kscd.kcfg)

kde4_add_executable(kscd ${kscd_SRCS})

target_link_libraries(kscd ${KDE4_KUTILS_LIBS} kcddb kcompactdisc)

########### install files ###############

install(TARGETS kscd DESTINATION ${BIN_INSTALL_DIR})
install( FILES ${resource_svg} DESTINATION ${DATA_INSTALL_DIR}/kscd/skin)

install(FILES kscd.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES kscd.profile.xml  DESTINATION  ${DATA_INSTALL_DIR}/profiles)
install(FILES kscd.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES audiocd_play.desktop  DESTINATION
${SERVICES_INSTALL_DIR}/ServiceMenus)
# install(PROGRAMS workman2cddb.pl DESTINATION ${BIN_INSTALL_DIR})

add_subdirectory(pics)

