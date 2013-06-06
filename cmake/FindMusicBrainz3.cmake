# Module to find the musicbrainz-3 library
#
# It defines
#  MUSICBRAINZ3_INCLUDE_DIR - the include dir 
#  MUSICBRAINZ3_LIBRARIES - the required libraries
#  MUSICBRAINZ3_FOUND - true if both of the above have been found

# Copyright (c) 2006,2007 Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(MUSICBRAINZ3_INCLUDE_DIR AND MUSICBRAINZ3_LIBRARIES)
   set(MUSICBRAINZ3_FIND_QUIETLY TRUE)
endif(MUSICBRAINZ3_INCLUDE_DIR AND MUSICBRAINZ3_LIBRARIES)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_check_modules(PC_LIBMUSICBRAINZ3 QUIET libmusicbrainz3)

FIND_PATH(MUSICBRAINZ3_INCLUDE_DIR musicbrainz3/musicbrainz.h
          HINTS
          ${PC_LIBMUSICBRAINZ3_INCLUDEDIR}
          ${PC_LIBMUSICBRAINZ3_INCLUDE_DIRS}
)

FIND_LIBRARY( MUSICBRAINZ3_LIBRARIES NAMES musicbrainz3
              HINTS
              ${PC_LIBMUSICBRAINZ3_LIBDIR}
              ${PC_LIBMUSICBRAINZ3_LIB_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( MusicBrainz3 DEFAULT_MSG
                                   MUSICBRAINZ3_INCLUDE_DIR MUSICBRAINZ3_LIBRARIES)

MARK_AS_ADVANCED(MUSICBRAINZ3_INCLUDE_DIR MUSICBRAINZ3_LIBRARIES)


