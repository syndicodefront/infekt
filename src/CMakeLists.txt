
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

find_package (OpenMP REQUIRED)
if (OPENMP_FOUND)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_subdirectory (console)
if (OPTION_GTK)
	add_subdirectory (gtk)
endif (OPTION_GTK)
