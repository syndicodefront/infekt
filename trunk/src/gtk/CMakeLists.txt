
include_directories (${INFEKT_SOURCE_DIR}/lib)
include_directories (${INFEKT_SOURCE_DIR}/lib-posix)
include_directories (${INFEKT_SOURCE_DIR}/gtk)

cmake_policy (SET CMP0011 NEW)

find_package(GTK2 2.16 REQUIRED gtk)

if(NOT GTK2_FOUND)
	message (FATAL_ERROR "Unable to locate GTK2.16 and gtkmm.")
endif()

#pkg_check_modules (GTK2 REQUIRED gtk+-2.0)
pkg_check_modules (GTKMM REQUIRED gtkmm-2.4)

set (LIBS ${LIBS} ${GTK2_LIBRARIES} ${GTKMM_LIBRARIES})
include_directories (${GTK2_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS})
link_directories (${GTK2_LIBRARY_DIRS} ${GTKMM_LIBRARY_DIRS})

add_executable (infekt-gtk
	main.cpp
	main_window.cpp
	nfo_view_ctrl.cpp
	${INFEKT_SOURCE_DIR}/lib/gutf8.c
	${INFEKT_SOURCE_DIR}/lib/nfo_data.cpp
	${INFEKT_SOURCE_DIR}/lib/nfo_renderer.cpp
#	${INFEKT_SOURCE_DIR}/lib/nfo_to_html.cpp
#	${INFEKT_SOURCE_DIR}/lib/nfo_to_pdf.cpp
	${INFEKT_SOURCE_DIR}/lib/util.cpp
	${INFEKT_SOURCE_DIR}/lib/cairo_box_blur.cpp
	${INFEKT_SOURCE_DIR}/lib-posix/infekt-posix.cpp
	${INFEKT_SOURCE_DIR}/lib-posix/iconv_string.c)

target_link_libraries (infekt-gtk ${LIBS})

