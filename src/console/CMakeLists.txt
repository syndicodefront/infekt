
include_directories (${INFEKT_SOURCE_DIR}/lib)
include_directories (${INFEKT_SOURCE_DIR}/lib-posix)
include_directories (${INFEKT_SOURCE_DIR}/console)

add_executable (infekt-cli infekt.cpp ${INFEKT_SOURCE_DIR}/lib/gutf8.c ${INFEKT_SOURCE_DIR}/lib/nfo_data.cpp ${INFEKT_SOURCE_DIR}/lib/nfo_renderer.cpp ${INFEKT_SOURCE_DIR}/lib/nfo_to_html.cpp ${INFEKT_SOURCE_DIR}/lib/nfo_to_pdf.cpp ${INFEKT_SOURCE_DIR}/lib/util.cpp ${INFEKT_SOURCE_DIR}/lib-posix/infekt-posix.cpp)

target_link_libraries (infekt-cli ${LIBS})

