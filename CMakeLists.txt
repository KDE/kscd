kde4_header()
project(kscd)

set(libworkmanaudio_SRCS 
	${CMAKE_SOURCE_DIR}/kscd/libwm/audio/audio.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/audio/audio_alsa.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/audio/audio_arts.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/audio/audio_sun.c
	)

set(libworkman_SRCS 
	${CMAKE_SOURCE_DIR}/kscd/libwm/cddb.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/cdinfo.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/cdrom.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/wm_helpers.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/cdtext.c
	${CMAKE_SOURCE_DIR}/kscd/libwm/database.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/index.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/scsi.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/cdda.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_linux_cdda.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_sun_cdda.c
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_aix.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_bsd386.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_freebsd.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_hpux.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_irix.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_linux.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_svr4.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_ultrix.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_news.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_openbsd.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_osf1.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_sun.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/plat_scor5.c
	${CMAKE_SOURCE_DIR}/kscd/libwm/drv_sony.c 
	${CMAKE_SOURCE_DIR}/kscd/libwm/drv_toshiba.c
)

add_subdirectory( bitmaps ) 
add_subdirectory( libwm ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(kscd_SRCS kscd.cpp ledlamp.cpp docking.cpp bwlednum.cpp  configWidget.cpp cddbdlg.cpp ${libkcompactdisc_SRCS} ${libworkmanaudio_SRCS} ${libworkman_SRCS})

kde4_add_ui3_files(kscd_SRCS configWidgetUI.ui panel.ui )

kde4_add_dcop_skels(kscd_SRCS kscd.h )

kde4_add_kcfg_files(kscd_SRCS prefs.kcfgc )

kde4_automoc(${kscd_SRCS})

kde4_add_executable(kscd ${kscd_SRCS})

target_link_libraries(kscd  ${KDE4_KIO_LIBS} kcddb )

install_targets(/bin kscd )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kscd.desktop )
install_files( ${DATA_INSTALL_DIR}/profiles FILES kscd.profile.xml )
install_files( ${KCFG_INSTALL_DIR} FILES kscd.kcfg )
install_files( ${DATA_INSTALL_DIR}/konqueror/servicemenus FILES audiocd_play.desktop )
install_files( ${DATA_INSTALL_DIR}/kscd/pics FILES  )
install_files( ${DATA_INSTALL_DIR}/kscd/icons FILES  )
install_files( ${MIME_INSTALL_DIR}/text FILES xmcd.desktop )
install_files( /bin FILES workman2cddb.pl)

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



#original Makefile.am contents follow:

## $Id: Makefile.am 439282 2005-07-27 15:42:10Z bmeyer $
#
#mimedir = $(kde_mimedir)/text
#picsdir  = $(kde_datadir)/kscd/pics
#
#INCLUDES= -I$(top_srcdir) -I$(top_builddir)/libkcddb $(all_includes)
#METASOURCES = AUTO
#
######## Files
#
#noinst_HEADERS = kscd.h bwlednum.h ledlamp.h \
#                 version.h docking.h configWidget.h
#
#kscd_SOURCES = panel.ui kscd.cpp kscd.skel ledlamp.cpp \
#               docking.cpp bwlednum.cpp \
#               configWidget.cpp configWidgetUI.ui prefs.kcfgc cddbdlg.cpp
#
#noinst_LTLIBRARIES = libkcompactdisc.la
#libkcompactdisc_la_SOURCES = kcompactdisc.cpp
#
#libkcompactdisc_la_LIBADD = $(top_builddir)/kscd/libwm/libworkman.la
#
#kde_kcfg_DATA = kscd.kcfg
#
#kscd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kscd_LDADD = $(LIBCDROM) $(LIBCDAUDIO) \
#	     $(LIBFPE) $(LIB_KIO) -lkutils $(top_builddir)/libkcddb/libkcddb.la \
#	     $(top_builddir)/kscd/libkcompactdisc.la
#
#profiledata_DATA = kscd.profile.xml
#profiledatadir = $(kde_datadir)/profiles
#
#servicedata_DATA = audiocd_play.desktop
#servicedatadir = $(kde_datadir)/konqueror/servicemenus
#
#DISTCLEANFILES = config.h
#
#bin_PROGRAMS = kscd
## Don't Build cddaslave. It doesn't work.
## cddaslave
#
##config.h: kscd-script
##$(SHELL) $(srcdir)/kscd-script
#
#SUBDIRS = bitmaps libwm
#
#bin_SCRIPTS = workman2cddb.pl
#
#kscdicondir = $(kde_datadir)/kscd/icons
#kscdicon_ICON = cdsmall
#KDE_ICON = kscd
#
#xdg_apps_DATA = kscd.desktop
#
#mime_DATA = xmcd.desktop
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kscd.pot
#
#cdrom.c:config.h
#plat_bsd386.c:config.h
#plat_freebsd.c:config.h
#plat_hpux.c:config.h
#plat_irix.c:config.h
#plat_linux.c:config.h
#plat_news.c:config.h
#plat_osf1.c:config.h
#plat_scor5.c:config.h
#plat_sun.c:config.h
#plat_sun_audio.c:config.h
#plat_sun_cdda.c:config.h
#plat_svr4.c:config.h
#plat_ultrix.c:config.h
#
#KDE_OPTIONS = nofinal
#
#
#cddbdlg.o: ../libkcddb/configbase.h ../libkcddb/cdinfodialogbase.h
#configWidget.o: ../libkcddb/configbase.h
#docking.o: ../libkcddb/configbase.h
#kscd.o: ../libkcddb/configbase.h
#kscd_skel.o: ../libkcddb/configbase.h panel.h prefs.h configWidgetUI.h
#
