cmake_minimum_required (VERSION 2.6)
project (INFEKT)

include (FindPkgConfig)
include (CheckIncludeFile)
include (CheckLibraryExists)

pkg_check_modules (CAIRO REQUIRED cairo)

set (LIBS ${LIBS} ${CAIRO_LIBRARIES})
include_directories (${CAIRO_INCLUDE_DIRS})
link_directories (${CAIRO_LIBRARY_DIRS})

message (STATUS "Looking for PCRE library")
find_library (PCRE_LIBRARY pcre)
if (NOT PCRE_LIBRARY)
	message (FATAL_ERROR "Unable to locate PCRE library.")
else (NOT PCRE_LIBRARY)
	message (STATUS "  found PCRE library")
endif (NOT PCRE_LIBRARY)

set (LIBS ${LIBS} pcre)

#CHECK_INCLUDE_FILE (pcre.h PCRE_HEADER)
#message (STATUS "test ${PCRE_LIBRARY} ${PCRE_HEADER} ${LIBS}")
#include_directories (PCRE_HEADER)

add_subdirectory (console)

