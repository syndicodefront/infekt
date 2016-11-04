
include (FindPkgConfig)
include (CheckIncludeFile)
include (CheckLibraryExists)

pkg_check_modules (CAIRO REQUIRED cairo)

set (LIBS ${LIBS} ${CAIRO_LIBRARIES})
include_directories (${CAIRO_INCLUDE_DIRS})
link_directories (${CAIRO_LIBRARY_DIRS})

pkg_check_modules (LIBPNG REQUIRED libpng)

set (LIBS ${LIBS} ${LIBPNG_LIBRARIES})
include_directories (${LIBPNG_INCLUDE_DIRS})
link_directories (${LIBPNG_LIBRARY_DIRS})

find_package (PNG REQUIRED)
if (PNG_FOUND)
       set (LIBS ${LIBS} ${PNG_LIBRARIES})
endif()

find_package (OpenMP REQUIRED)
if (OPENMP_FOUND)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_subdirectory (console)
if (OPTION_GTK)
	add_subdirectory (gtk)
endif (OPTION_GTK)
