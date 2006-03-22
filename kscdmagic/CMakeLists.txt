kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kscdmagic_SRCS 
   core.cpp 
   main.cpp 
   sound.cpp 
   xlibwrap.cpp 
   xlib.c )

kde4_automoc(${kscdmagic_SRCS})

kde4_add_executable(kscdmagic ${kscdmagic_SRCS})

target_link_libraries(kscdmagic  ${KDE4_KDECORE_LIBS} m kdeui )

install_targets(/bin kscdmagic )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#
## this 10 paths are KDE specific. Use them:
## kde_htmldir       Where your docs should go to. (contains lang subdirs)
## kde_appsdir       Where your application file (.kdelnk) should go to. 
## kde_icondir       Where your icon should go to.
## kde_minidir       Where your mini icon should go to.
## kde_datadir       Where you install application data. (Use a subdir)
## kde_locale        Where translation files should go to.(contains lang subdirs)
## kde_cgidir        Where cgi-bin executables should go to.
## kde_confdir       Where config files should go to.
## kde_mimedir       Where mimetypes should go to.
## kde_toolbardir    Where general toolbar icons should go to.
## kde_wallpaperdir  Where general wallpapers should go to.
#
## just set the variable
#APPSDIR = $(kde_appsdir)/Multimedia
#
## set the include path for X, qt and KDE
#INCLUDES= $(all_includes)
#
######## This part is very kscdmagic specific
## you can add here more. This one gets installed 
#bin_PROGRAMS = 	kscdmagic
#
## Which sources should be compiled for kscdmagic
#kscdmagic_SOURCES = core.cpp main.cpp sound.cpp xlibwrap.cpp xlib.c
#
## the library search path. 
#kscdmagic_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
## the libraries to link against. Be aware of the order. First the libraries,
## that depend on the following ones.
#kscdmagic_LDADD   =       -lm $(LIB_KDEUI) 
#
## this option you can leave out. Just, if you use "make dist", you need it
#noinst_HEADERS =  symbol.h    syna.h      xlib.h magicconf.h
#
#messages:
##	$(XGETTEXT) -kTRANS $(kscdmagic_SOURCES)
#
