# $Id$

mimedir = $(kde_mimedir)/text
picsdir  = $(kde_datadir)/kscd/pics

INCLUDES= -I$(top_srcdir) $(all_includes)
METASOURCES = AUTO

####### Files

noinst_HEADERS = kscd.h bwlednum.h ledlamp.h inexact.h \
	CDDialog.h CDDialogData.h CDDBSetup.h CDDBSetupData.h mgconfdlg.h \
	configdlg.h version.h docking.h smtpconfig.h \
	smtp.h configWidget.h kvolumecontrol.h

kscd_SOURCES = kvolumecontrol.cpp panel.ui kscd.cpp kscd.skel configdlg.cpp ledlamp.cpp CDDialog.cpp \
	       CDDialogData.cpp CDDBSetup.cpp CDDBSetupData.cpp docking.cpp \
	       smtpconfig.cpp smtp.cpp inexact.cpp mgconfdlg.cpp  bwlednum.cpp \
	       configWidget.cpp configWidgetUI.ui

kscd_LDFLAGS = $(all_libraries) -lkutils
kscd_LDADD = $(top_builddir)/kscd/libwm/libworkman.la \
             $(LIBCDROM) $(LIBCDAUDIO) $(LIBFPE) $(LIB_KIO) $(top_builddir)/libkcddb/libkcddb.la

profiledata_DATA = kscd.profile.xml
profiledatadir = $(kde_datadir)/profiles

DISTCLEANFILES = config.h

bin_PROGRAMS = kscd
# Don't Build cddaslave. It doesn't work.
# cddaslave

#config.h: kscd-script
#$(SHELL) $(srcdir)/kscd-script

SUBDIRS = bitmaps libwm

bin_SCRIPTS = workman2cddb.pl

kscdicondir = $(kde_datadir)/kscd/icons
kscdicon_ICON = cdsmall
KDE_ICON = kscd

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

