project(kscd)

include_directories(
                ${CMAKE_CURRENT_SOURCE_DIR}/gui
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
# 	cddbdlg.cpp 
	mbmanager.cpp
	gui/kscdwindow.cpp 
	gui/kscdwidget.cpp 
	gui/ejectbutton.cpp 
	gui/stopbutton.cpp 
	gui/playbutton.cpp 
	gui/nextbutton.cpp 
	gui/previousbutton.cpp 
	gui/mutebutton.cpp
	gui/randombutton.cpp
	gui/loopbutton.cpp
	gui/tracklistbutton.cpp
	gui/volumebutton.cpp
	gui/panel.cpp
	gui/titlePopUp.cpp
	gui/tracklistdlg.cpp
	gui/background.cpp
	gui/closebutton.cpp
	gui/minimizebutton.cpp
	gui/finderSkin.cpp
	gui/seekslider.cpp
	gui/seekcursor.cpp
	gui/seekbar.cpp
)

file(GLOB resource_svg "gui/skin/*.svg")

kde4_add_ui_files(kscd_SRCS gui/trackListDlgUI.ui gui/finderSkinUI.ui generalSettings.ui interfaceSettings.ui)

kde4_add_kcfg_files(kscd_SRCS prefs.kcfgc)

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

add_subdirectory(pics)

