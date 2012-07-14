project(kscd)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

find_package(Alsa)
alsa_configure_file(${CMAKE_CURRENT_BINARY_DIR}/config-alsa.h)

find_package(MusicBrainz3 REQUIRED)

include_directories(
                ${CMAKE_CURRENT_SOURCE_DIR}/gui
		${CMAKE_CURRENT_SOURCE_DIR}
			${CMAKE_CURRENT_BINARY_DIR}
		)
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

# For MusicBrainz error handling
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

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
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4_PHONON_LIBS}
    ${MUSICBRAINZ3_LIBRARIES}
    )

########### install files ###############

install(TARGETS kscd ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES ${resource_svg} DESTINATION ${DATA_INSTALL_DIR}/kscd/skin)

install(PROGRAMS kscd.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR})
install(FILES kscd.kcfg  DESTINATION  ${KCFG_INSTALL_DIR})
install(FILES kscd-play-audiocd.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions )

install( FILES org.kde.kscd.cdplayer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

add_subdirectory(pics)

