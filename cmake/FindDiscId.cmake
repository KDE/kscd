# Module to find the discid library
#
# It defines
#  DISCID_INCLUDE_DIR - the include dir
#  DISCID_LIBRARIES - the required libraries
#  DISCID_FOUND - true if both of the above have been found

# Copyright (c) 2006,2007 Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(DISCID_INCLUDE_DIR AND DISCID_LIBRARIES)
   set(DISCID_FIND_QUIETLY TRUE)
endif(DISCID_INCLUDE_DIR AND DISCID_LIBRARIES)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_check_modules(PC_LIBDISCID QUIET discid)

FIND_PATH(DISCID_INCLUDE_DIR discid/discid.h
          HINTS
          ${PC_LIBDISCID_INCLUDEDIR}
          ${PC_LIBDISCID_INCLUDE_DIRS}
)

FIND_LIBRARY( DISCID_LIBRARIES NAMES discid
              HINTS
              ${PC_LIBDISCID_LIBDIR}
              ${PC_LIBDISCID_LIB_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( DiscId DEFAULT_MSG
                                   DISCID_INCLUDE_DIR DISCID_LIBRARIES)

MARK_AS_ADVANCED(DISCID_INCLUDE_DIR DISCID_LIBRARIES)


