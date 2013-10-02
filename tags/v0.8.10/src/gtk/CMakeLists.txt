
include_directories (${INFEKT_SOURCE_DIR}/src/lib)
include_directories (${INFEKT_SOURCE_DIR}/src/lib-posix)
include_directories (${INFEKT_SOURCE_DIR}/src/gtk)

cmake_policy (SET CMP0011 NEW)

find_package(GTK2 2.16 REQUIRED gtk)

if(NOT GTK2_FOUND)
	message (FATAL_ERROR "Unable to locate GTK2.16.")
endif()

pkg_check_modules (GTKMM REQUIRED gtkmm-2.4)

set (LIBS ${LIBS} ${GTK2_LIBRARIES} ${GTKMM_LIBRARIES})
include_directories (${GTK2_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS})
link_directories (${GTK2_LIBRARY_DIRS} ${GTKMM_LIBRARY_DIRS})

configure_file ("${INFEKT_SOURCE_DIR}/src/gtk/infekt-build-conf.h.in"
	"${INFEKT_BINARY_DIR}/infekt-build-conf.h")
include_directories ("${INFEKT_BINARY_DIR}")

add_executable (infekt-gtk
	main.cpp
	main_window.cpp
	nfo_view_ctrl.cpp
	${INFEKT_SOURCE_DIR}/src/lib/gutf8.c
	${INFEKT_SOURCE_DIR}/src/lib/nfo_data.cpp
	${INFEKT_SOURCE_DIR}/src/lib/nfo_renderer.cpp
	${INFEKT_SOURCE_DIR}/src/lib/util.cpp
	${INFEKT_SOURCE_DIR}/src/lib/cairo_box_blur.cpp
	${INFEKT_SOURCE_DIR}/src/lib-posix/infekt-posix.cpp
	${INFEKT_SOURCE_DIR}/src/lib-posix/iconv_string.c)

target_link_libraries (infekt-gtk ${LIBS})

install (PROGRAMS ${INFEKT_BINARY_DIR}/src/gtk/infekt-gtk DESTINATION bin)
install (FILES ${INFEKT_SOURCE_DIR}/src/gtk/infektwindowdata.glade DESTINATION share/infekt) 
