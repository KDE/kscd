project(kscd)

cmake_minimum_required(VERSION 2.8.12)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${ECM_MODULE_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_definitions(-fexceptions)

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)

# Handle build type
if(CMAKE_BUILD_TYPE MATCHES [Dd]ebug)
    message(STATUS "Debug build")
    add_definitions(-DKSCD_DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
    message(STATUS "Release build")
    add_definitions(-DNDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets Svg)
find_package(KF5 REQUIRED I18n Solid KIO KDELibs4Support GlobalAccel DBusAddons)
find_package(Phonon4Qt5 REQUIRED)
find_package(MusicBrainz3 REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
    ${PHONON_INCLUDE_DIR}
	${MUSICBRAINZ3_INCLUDE_DIR}
)
if(BUILD_TESTING)
    find_package(Qt5Test REQUIRED)
    add_subdirectory(tests)
endif()
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

qt5_add_dbus_adaptor(kscd_SRCS org.kde.kscd.cdplayer.xml kscd.h KSCD)
qt5_add_dbus_adaptor( kscd_SRCS dbus/org.freedesktop.MediaPlayer.root.xml dbus/RootDBusHandler.h KsCD::RootDBusHandler RootAdaptor RootAdaptor)
qt5_add_dbus_adaptor( kscd_SRCS dbus/org.freedesktop.MediaPlayer.player.xml dbus/PlayerDBusHandler.h KsCD::PlayerDBusHandler PlayerAdaptor PlayerAdaptor)
qt5_add_dbus_adaptor( kscd_SRCS dbus/org.freedesktop.MediaPlayer.tracklist.xml dbus/TracklistDBusHandler.h KsCD::TracklistDBusHandler TracklistAdaptor TracklistAdaptor)

##################################
#        Graphics Setting        #
##################################

file(GLOB resource_svg "gui/skin/*.svg")

ki18n_wrap_ui(kscd_SRCS gui/trackListDlgUI.ui generalSettings.ui interfaceSettings.ui)

##################################
#        KsCD Executable         #
##################################

kconfig_add_kcfg_files(kscd_SRCS prefs.kcfgc)

add_executable(kscd ${kscd_SRCS})

##################################
#        Libraries Linking       #
##################################

target_link_libraries(kscd
    Qt5::Widgets
    Qt5::Svg
    KF5::Solid
    KF5::KDELibs4Support
    KF5::GlobalAccel
    KF5::DBusAddons
    ${PHONON_LIBRARIES}
    ${MUSICBRAINZ3_LIBRARIES}
)

########### install files ###############

install(TARGETS kscd ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${resource_svg} DESTINATION ${DATA_INSTALL_DIR}/kscd/skin)

install(PROGRAMS org.kde.kscd.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kscd.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kscd-play-audiocd.desktop DESTINATION ${DATA_INSTALL_DIR}/solid/actions)

install(FILES org.kde.kscd.cdplayer.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

add_subdirectory(pics)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
