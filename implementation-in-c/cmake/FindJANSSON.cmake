# try to find libjansson, once done following variables will be defined
#
# JANSSON_FOUND       - system has libjansson
# JANSSON_VERSION     - the version of libansson
# JANSSON_INCLUDE_DIR - the libjansson include directory
# JANSSON_LIBRARY     - the filepath of libjansson.a|so|dylib


if (JANSSON_INCLUDE_DIR AND JANSSON_LIBRARY)
    set(JANSSON_FOUND TRUE)
else()
    pkg_check_modules(PKG_CONFIG_JANSSON QUIET jansson)

    message("PKG_CONFIG_JANSSON_FOUND=${PKG_CONFIG_JANSSON_FOUND}")
    message("PKG_CONFIG_JANSSON_INCLUDE_DIRS=${PKG_CONFIG_JANSSON_INCLUDE_DIRS}")
    message("PKG_CONFIG_JANSSON_LIBRARY_DIRS=${PKG_CONFIG_JANSSON_LIBRARY_DIRS}")
    message("PKG_CONFIG_JANSSON_INCLUDEDIR=${PKG_CONFIG_JANSSON_INCLUDEDIR}")
    message("PKG_CONFIG_JANSSON_LIBDIR=${PKG_CONFIG_JANSSON_LIBDIR}")
    message("PKG_CONFIG_JANSSON_VERSION=${PKG_CONFIG_JANSSON_VERSION}")
    message("PKG_CONFIG_JANSSON_LIBRARIES=${PKG_CONFIG_JANSSON_LIBRARIES}")
    message("PKG_CONFIG_JANSSON_LINK_LIBRARIES=${PKG_CONFIG_JANSSON_LINK_LIBRARIES}")
    message("PKG_CONFIG_JANSSON_STATIC_LIBRARIES=${PKG_CONFIG_JANSSON_STATIC_LIBRARIES}")

    if (PKG_CONFIG_JANSSON_FOUND)
        if (PKG_CONFIG_JANSSON_INCLUDE_DIRS)
            set(JANSSON_INCLUDE_DIR "${PKG_CONFIG_JANSSON_INCLUDE_DIRS}")
        elseif (PKG_CONFIG_JANSSON_INCLUDEDIR)
            set(JANSSON_INCLUDE_DIR "${PKG_CONFIG_JANSSON_INCLUDEDIR}")
        else()
            find_path(JANSSON_INCLUDE_DIR jansson.h)
        endif()

        set(JANSSON_LIBRARY     "${PKG_CONFIG_JANSSON_LINK_LIBRARIES}")
    else()
        find_path   (JANSSON_INCLUDE_DIR jansson.h)
        find_library(JANSSON_LIBRARY     jansson)
    endif()
    
    if (JANSSON_INCLUDE_DIR)
        if (PKG_CONFIG_JANSSON_VERSION)
            set(JANSSON_VERSION ${PKG_CONFIG_JANSSON_VERSION})
        else()
	        file(STRINGS "${JANSSON_INCLUDE_DIR}/jansson.h" JANSSON_VERSION REGEX "^#define[\t ]+JANSSON_VERSION[\t ]+\".*\"")
	        string(REGEX REPLACE "^#define[\t ]+JANSSON_VERSION[\t ]+\"([^\"]*)\".*" "\\1" JANSSON_VERSION "${JANSSON_VERSION}")
        endif()
    endif()
endif()

if (NOT TARGET  JANSSON::JANSSON)
    add_library(JANSSON::JANSSON UNKNOWN IMPORTED)
    set_target_properties(JANSSON::JANSSON PROPERTIES
        IMPORTED_LOCATION             "${JANSSON_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${JANSSON_INCLUDE_DIR}"
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JANSSON REQUIRED_VARS JANSSON_LIBRARY JANSSON_INCLUDE_DIR VERSION_VAR JANSSON_VERSION)

mark_as_advanced(JANSSON_INCLUDE_DIR JANSSON_LIBRARY)
