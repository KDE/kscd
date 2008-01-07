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

set(kscd_SRCS 
	kscd.cpp 
	docking.cpp 
	hwcontroler.cpp 
	audiocd.cpp 
	bwlednum.cpp 
	configWidget.cpp 
	cddbdlg.cpp 
	cdcombobox.cpp
	ihm/kscdwindow.cpp 
	ihm/kscdwidget.cpp 
	ihm/stopbutton.cpp 
	ihm/playbutton.cpp 
	ihm/nextbutton.cpp 
	ihm/previousbutton.cpp 
	ihm/ejectbutton.cpp 
	ihm/mutebutton.cpp
	ihm/randombutton.cpp
	ihm/loopbutton.cpp
	ihm/tracklistbutton.cpp
	ihm/volumebutton.cpp)

set(kscd_Resources ihm/skin)


kde4_add_ui_files(kscd_SRCS panel.ui configWidgetUI.ui)

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

set(kscdtest_SRCS 
    ${kscd_SRCS} 
    tests/audiocd_test.cpp 
    tests/hwcontroler_test.cpp 
    tests/test.cpp 
    ) 
 
KDE4_ADD_UNIT_TEST(kscdtest ${kscdtest_SRCS}) 
TARGET_LINK_LIBRARIES(kscdtest ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY})

