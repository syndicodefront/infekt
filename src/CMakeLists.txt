
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

if(APPLE)
    find_library(ICONV_LIBRARY iconv REQUIRED)
    set(LIBS ${LIBS} ${ICONV_LIBRARY})

    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)

    execute_process(COMMAND brew --prefix libomp OUTPUT_VARIABLE HOMEBREW_LIBOMP_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(CMAKE_C_COMPILER_ID MATCHES "Clang\$")
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY "${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib")
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang\$")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${HOMEBREW_LIBOMP_PREFIX}/include")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY "${HOMEBREW_LIBOMP_PREFIX}/lib/libomp.dylib")
    endif()
endif()

find_package(OpenMP REQUIRED)

if (OPENMP_FOUND)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_subdirectory (console)
if (OPTION_GTK)
	add_subdirectory (gtk)
endif (OPTION_GTK)
