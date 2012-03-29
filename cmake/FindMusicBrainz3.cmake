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

FIND_PATH(MUSICBRAINZ3_INCLUDE_DIR musicbrainz3/musicbrainz.h)

FIND_LIBRARY( MUSICBRAINZ3_LIBRARIES NAMES musicbrainz3)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( MusicBrainz3 DEFAULT_MSG
                                   MUSICBRAINZ3_INCLUDE_DIR MUSICBRAINZ3_LIBRARIES)

MARK_AS_ADVANCED(MUSICBRAINZ3_INCLUDE_DIR MUSICBRAINZ3_LIBRARIES)


