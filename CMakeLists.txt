project(kscd)

find_package(MusicBrainz REQUIRED)

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

set(kscd_SRCS 
	kscd.cpp 
# 	docking.cpp 
	hwcontroler.cpp 
	audiocd.cpp 
# 	bwlednum.cpp 
# 	configWidget.cpp 
# 	cddbdlg.cpp 
	cddbmanager.cpp
	mbmanager.cpp
# 	cdcombobox.cpp
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
	ihm/configwindow.cpp
# 	ihm/titlePopUp.cpp
	ihm/tracklistdlg.cpp
	ihm/background.cpp
	ihm/closebutton.cpp
	ihm/minimizebutton.cpp
)

set(kscd_Resources ihm/skin)

kde4_add_ui_files(kscd_SRCS ihm/trackListDlgUI.ui)

kde4_add_kcfg_files(kscd_SRCS prefs.kcfgc)


kde4_add_executable(kscd ${kscd_SRCS})

target_link_libraries(kscd ${KDE4_KUTILS_LIBS} kcddb kcompactdisc)

install(TARGETS kscd DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############

install(TARGETS kscd DESTINATION ${BIN_INSTALL_DIR})
install(DIRECTORY ${kscd_Resources} DESTINATION ${DATA_INSTALL_DIR}/kscd)

install(FILES kscd.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES kscd.profile.xml  DESTINATION  ${DATA_INSTALL_DIR}/profiles)
install(FILES kscd.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES audiocd_play.desktop  DESTINATION
${SERVICES_INSTALL_DIR}/ServiceMenus)
install(PROGRAMS workman2cddb.pl DESTINATION ${BIN_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})


############ Tests ##############

# HWControler test unit
kde4_add_unit_test(HWControler_test
	hwcontroler.cpp
	audiocd.cpp
	./tests/hwcontroler_test.cpp
)

# libraries needed for HWControler tests
target_link_libraries(HWControler_test
	${KDE4_KUTILS_LIBS}
	${QT_QTTEST_LIBRARY}
	kcddb
	kcompactdisc
)


