if (NOT NDK_PKG_HOME)
    set(NDK_PKG_HOME "$ENV{HOME}/.ndk-pkg")
endif()

message(STATUS "NDK_PKG_HOME=${NDK_PKG_HOME}")

if (NOT IS_DIRECTORY "${NDK_PKG_HOME}/install.d")
    message(FATAL_ERROR "no any ndk-pkg packages installed.")
endif()

file(GLOB items LIST_DIRECTORIES true "${NDK_PKG_HOME}/install.d/*")
foreach(item ${items})
    #message(STATUS "ndk-pkg package: ${item}")
    if ((IS_DIRECTORY ${item}) AND (EXISTS "${item}/installed-abis") AND (EXISTS "${item}/installed-metadata"))
        if (IS_DIRECTORY "${item}/${ANDROID_ABI}")
            list(APPEND CMAKE_FIND_ROOT_PATH "${item}/${ANDROID_ABI}")
        else()
            message(FATAL_ERROR "${item}/${ANDROID_ABI} not exsits.")
        endif()

        if (EXISTS "${item}/${ANDROID_ABI}/bin/")
            list(APPEND CMAKE_IGNORE_PATH "${item}/${ANDROID_ABI}/bin")
        endif()

        if (EXISTS "${item}/${ANDROID_ABI}/sbin/")
            list(APPEND CMAKE_IGNORE_PATH "${item}/${ANDROID_ABI}/sbin")
        endif()

        if (EXISTS "${item}/${ANDROID_ABI}/lib/pkgconfig")
            set(PKG_CONFIG_LIBDIR "${item}/${ANDROID_ABI}/lib/pkgconfig:${PKG_CONFIG_LIBDIR}")
        endif()
    endif()
endforeach()

set(ENV{PKG_CONFIG_LIBDIR} "${PKG_CONFIG_LIBDIR}")

#message(STATUS "CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}")
#message(STATUS "CMAKE_IGNORE_PATH=${CMAKE_IGNORE_PATH}")
#message(STATUS "PKG_CONFIG_LIBDIR=$ENV{PKG_CONFIG_LIBDIR}")
