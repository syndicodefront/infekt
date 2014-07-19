
include_directories (${INFEKT_SOURCE_DIR}/src/lib)
include_directories (${INFEKT_SOURCE_DIR}/src/lib-posix)
include_directories (${INFEKT_SOURCE_DIR}/src/console)

add_executable (infekt-cli
	infekt.cpp
	${INFEKT_SOURCE_DIR}/src/lib/gutf8.c
	${INFEKT_SOURCE_DIR}/src/lib/nfo_data.cpp
	${INFEKT_SOURCE_DIR}/src/lib/nfo_hyperlink.cpp
	${INFEKT_SOURCE_DIR}/src/lib/ansi_art.cpp
	${INFEKT_SOURCE_DIR}/src/lib/nfo_colormap.cpp
	${INFEKT_SOURCE_DIR}/src/lib/nfo_renderer.cpp
	${INFEKT_SOURCE_DIR}/src/lib/nfo_to_html.cpp
	${INFEKT_SOURCE_DIR}/src/lib/nfo_to_pdf.cpp
	${INFEKT_SOURCE_DIR}/src/lib/nfo_to_png.cpp
	${INFEKT_SOURCE_DIR}/src/lib/util.cpp
	${INFEKT_SOURCE_DIR}/src/lib/cairo_box_blur.cpp
	${INFEKT_SOURCE_DIR}/src/lib-posix/infekt-posix.cpp
	${INFEKT_SOURCE_DIR}/src/lib-posix/iconv_string.c)

target_link_libraries (infekt-cli ${LIBS})

install (PROGRAMS ${INFEKT_BINARY_DIR}/src/console/infekt-cli DESTINATION bin)
