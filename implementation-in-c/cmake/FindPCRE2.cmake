# try to find pcre2, once done following variables will be defined
#
# PCRE2_FOUND       - system has pcre2
# PCRE2_VERSION     - the version of pcre2
# PCRE2_INCLUDE_DIR - the pcre2 include directory
# PCRE2_LIBRARY     - the filepath of libpcre2-8.a|so|dylib


if (PCRE2_INCLUDE_DIR AND PCRE2_LIBRARY)
    set(PCRE2_FOUND TRUE)
else()
    pkg_check_modules(PKG_CONFIG_PCRE2 QUIET pcre2-8)

    message("PKG_CONFIG_PCRE2_FOUND=${PKG_CONFIG_PCRE2_FOUND}")
    message("PKG_CONFIG_PCRE2_INCLUDE_DIRS=${PKG_CONFIG_PCRE2_INCLUDE_DIRS}")
    message("PKG_CONFIG_PCRE2_LIBRARY_DIRS=${PKG_CONFIG_PCRE2_LIBRARY_DIRS}")
    message("PKG_CONFIG_PCRE2_INCLUDEDIR=${PKG_CONFIG_PCRE2_INCLUDEDIR}")
    message("PKG_CONFIG_PCRE2_LIBDIR=${PKG_CONFIG_PCRE2_LIBDIR}")
    message("PKG_CONFIG_PCRE2_VERSION=${PKG_CONFIG_PCRE2_VERSION}")
    message("PKG_CONFIG_PCRE2_LIBRARIES=${PKG_CONFIG_PCRE2_LIBRARIES}")
    message("PKG_CONFIG_PCRE2_LINK_LIBRARIES=${PKG_CONFIG_PCRE2_LINK_LIBRARIES}")
    message("PKG_CONFIG_PCRE2_STATIC_LIBRARIES=${PKG_CONFIG_PCRE2_STATIC_LIBRARIES}")

    if (PKG_CONFIG_PCRE2_FOUND)
        if (PKG_CONFIG_PCRE2_INCLUDE_DIRS)
            set(PCRE2_INCLUDE_DIR "${PKG_CONFIG_PCRE2_INCLUDE_DIRS}")
        elseif (PKG_CONFIG_PCRE2_INCLUDEDIR)
            set(PCRE2_INCLUDE_DIR "${PKG_CONFIG_PCRE2_INCLUDEDIR}")
        else()
            find_path(PCRE2_INCLUDE_DIR pcre2.h)
        endif()

        set(PCRE2_LIBRARY     "${PKG_CONFIG_PCRE2_LINK_LIBRARIES}")
    else()
        find_path   (PCRE2_INCLUDE_DIR pcre2.h)
        find_library(PCRE2_LIBRARY     pcre2-8)
    endif()

    if (PKG_CONFIG_PCRE2_VERSION)
        set(PCRE2_VERSION ${PKG_CONFIG_PCRE2_VERSION})
    endif()
endif()

if (NOT TARGET  PCRE2::PCRE2)
    add_library(PCRE2::PCRE2 UNKNOWN IMPORTED)
    set_target_properties(PCRE2::PCRE2 PROPERTIES
        IMPORTED_LOCATION             "${PCRE2_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${PCRE2_INCLUDE_DIR}"
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE2 REQUIRED_VARS PCRE2_LIBRARY PCRE2_INCLUDE_DIR VERSION_VAR PCRE2_VERSION)

mark_as_advanced(PCRE2_INCLUDE_DIR PCRE2_LIBRARY)
