# $Id$

mimedir = $(kde_mimedir)/text
picsdir  = $(kde_datadir)/kscd/pics

INCLUDES= -I$(top_srcdir) $(all_includes)

####### Files

noinst_HEADERS = kscd.h bwlednum.h ledlamp.h inexact.h \
	CDDialog.h CDDialogData.h CDDBSetup.h CDDBSetupData.h mgconfdlg.h \
	configdlg.h version.h docking.h smtpconfig.h \
	smtp.h configWidget.h

#	struct.h wmcdda.h

kscd_SOURCES = kscd.cpp kscd.skel configdlg.cpp ledlamp.cpp CDDialog.cpp \
	CDDialogData.cpp CDDBSetup.cpp CDDBSetupData.cpp docking.cpp \
	smtpconfig.cpp smtp.cpp inexact.cpp mgconfdlg.cpp  bwlednum.cpp \
        configWidget.cpp configWidgetUI.ui

kscd_LDFLAGS = $(all_libraries) -lkutils
kscd_LDADD = $(top_builddir)/kscd/libwm/libworkman.la \
             $(LIBCDROM) $(LIBCDAUDIO) $(LIBFPE) $(LIB_KIO) $(top_builddir)/libkcddb/libkcddb.la

profiledata_DATA = kscd.profile.xml
profiledatadir = $(kde_datadir)/profiles

#SOURCES3=kscdmagic/bitmap.cpp  kscdmagic/core.cpp kscdmagic/main.cpp
#	kscdmagic/sound.cpp kscdmagic/xlibwrap.cpp kscdmagic/xlib.c

#OBJECTS3=kscdmagic/bitmap.o  kscdmagic/core.o kscdmagic/main.o
#	kscdmagic/sound.o kscdmagic/xlibwrap.o kscdmagic/xlib.o

METASOURCES = kscd.moc bwlednum.moc configdlg.moc ledlamp.moc \
	 inexact.moc CDDialog.moc CDDialogData.moc CDDBSetup.moc \
	CDDBSetupData.moc docking.moc mgconfdlg.moc smtpconfig.moc \
	smtp.moc

DISTCLEANFILES = config.h

bin_PROGRAMS = kscd
# Don't Build cddaslave. It doesn't work.
# cddaslave
#TARGET3 = kscdmagic

#config.h: kscd-script
#$(SHELL) $(srcdir)/kscd-script

SUBDIRS = bitmaps libwm

bin_SCRIPTS = workman2cddb.pl

kscdicondir = $(kde_datadir)/kscd/icons
kscdicon_ICON = AUTO

xdg_apps_DATA = kscd.desktop

mime_DATA = xmcd.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kscd.pot

cdrom.c:config.h
plat_bsd386.c:config.h
plat_freebsd.c:config.h
plat_hpux.c:config.h
plat_irix.c:config.h
plat_linux.c:config.h
plat_news.c:config.h
plat_osf1.c:config.h
plat_scor5.c:config.h
plat_sun.c:config.h
plat_sun_audio.c:config.h
plat_sun_cdda.c:config.h
plat_svr4.c:config.h
plat_ultrix.c:config.h

