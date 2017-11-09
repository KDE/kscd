# Module to find the musicbrainz-5 library
#
# It defines
#  MUSICBRAINZ5_INCLUDE_DIR - the include dir
#  MUSICBRAINZ5_LIBRARIES - the required libraries
#  MUSICBRAINZ5_FOUND - true if both of the above have been found

# Copyright (c) 2006,2007 Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(MUSICBRAINZ5_INCLUDE_DIR AND MUSICBRAINZ5_LIBRARIES)
   set(MUSICBRAINZ5_FIND_QUIETLY TRUE)
endif(MUSICBRAINZ5_INCLUDE_DIR AND MUSICBRAINZ5_LIBRARIES)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig)
pkg_check_modules(PC_LIBMUSICBRAINZ5 QUIET libmusicbrainz5cc libmusicbrainz5)

FIND_PATH(MUSICBRAINZ5_INCLUDE_DIR musicbrainz5/Query.h
          HINTS
          ${PC_LIBMUSICBRAINZ5_INCLUDEDIR}
          ${PC_LIBMUSICBRAINZ5_INCLUDE_DIRS}
)

FIND_LIBRARY( MUSICBRAINZ5_LIBRARIES NAMES musicbrainz5cc musicbrainz5
              HINTS
              ${PC_LIBMUSICBRAINZ5_LIBDIR}
              ${PC_LIBMUSICBRAINZ5_LIB_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( MusicBrainz5 DEFAULT_MSG
                                   MUSICBRAINZ5_INCLUDE_DIR MUSICBRAINZ5_LIBRARIES)

MARK_AS_ADVANCED(MUSICBRAINZ5_INCLUDE_DIR MUSICBRAINZ5_LIBRARIES)


