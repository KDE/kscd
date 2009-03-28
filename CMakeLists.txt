project(kscd)

include_directories(
                ${CMAKE_CURRENT_SOURCE_DIR}/gui
		${CMAKE_SOURCE_DIR}/kscd
			${CMAKE_BINARY_DIR}/kscd
		)
add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_subdirectory(tests)
########### next target ###############

set(kscd_SRCS 

##########
# Main
##########

	kscd.cpp 
	
###############
# Solid/Phonon
###############

	hwcontroler.cpp 
	audiocd.cpp 

#########################
# Music Brainz 
#########################

	mbmanager.cpp
	
####################
# Graphics / SVG
####################

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
	#gui/seekslider.cpp
	gui/seekcursor.cpp
	gui/seekbar.cpp
	
####################
# DBus Connection
####################

	dbus/PlayerDBusHandler.cpp
	dbus/RootDBusHandler.cpp
	dbus/TracklistDBusHandler.cpp
)


##################################
#        DBUS LINKING            #
##################################

qt4_add_dbus_adaptor(kscd_SRCS org.kde.kscd.cdplayer.xml kscd.h KSCD)
qt4_add_dbus_adaptor( kscd_SRCS dbus/org.freedesktop.MediaPlayer.root.xml dbus/RootDBusHandler.h KsCD::RootDBusHandler RootAdaptor RootAdaptor)
qt4_add_dbus_adaptor( kscd_SRCS dbus/org.freedesktop.MediaPlayer.player.xml dbus/PlayerDBusHandler.h KsCD::PlayerDBusHandler PlayerAdaptor PlayerAdaptor)
qt4_add_dbus_adaptor( kscd_SRCS dbus/org.freedesktop.MediaPlayer.tracklist.xml dbus/TracklistDBusHandler.h KsCD::TracklistDBusHandler TracklistAdaptor TracklistAdaptor)

##################################
#        Graphics Setting        #
##################################

file(GLOB resource_svg "gui/skin/*.svg")

kde4_add_ui_files(kscd_SRCS gui/trackListDlgUI.ui generalSettings.ui interfaceSettings.ui)

##################################
#        KsCD Executable         #
##################################

kde4_add_kcfg_files(kscd_SRCS prefs.kcfgc)

kde4_add_executable(kscd ${kscd_SRCS})

##################################
#        Libraries Linking       #
##################################

target_link_libraries(kscd
    ${KDE4_KUTILS_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4_PHONON_LIBS}
    ${MUSICBRAINZ_LIBRARIES}
    kcddb
    kcompactdisc)

########### install files ###############

install(TARGETS kscd ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ${resource_svg} DESTINATION ${DATA_INSTALL_DIR}/kscd/skin)

install(FILES kscd.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES kscd.profile.xml  DESTINATION  ${DATA_INSTALL_DIR}/profiles)
install(FILES kscd.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES kscd-play-audiocd.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions )

install( FILES org.kde.kscd.cdplayer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

add_subdirectory(pics)

