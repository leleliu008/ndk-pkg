# https://developer.android.com/ndk/guides/cmake

if (NOT NDK_PKG_HOME)
    set(NDK_PKG_HOME "$ENV{HOME}/.ndk-pkg")
endif()

message(STATUS "NDK_PKG_HOME=${NDK_PKG_HOME}")
message(STATUS "NDK_PKG_ROOT=${NDK_PKG_HOME}/install.d/android/${ANDROID_PLATFORM_LEVEL}")

if (NOT IS_DIRECTORY "${NDK_PKG_HOME}/install.d/android/${ANDROID_PLATFORM_LEVEL}")
    message(FATAL_ERROR "no any ndk-pkg packages installed.")
endif()

file(GLOB items LIST_DIRECTORIES true "${NDK_PKG_HOME}/install.d/android/${ANDROID_PLATFORM_LEVEL}/*")
foreach(item ${items})
    if ((IS_DIRECTORY ${item}) AND (EXISTS "${item}/installed-abis") AND (EXISTS "${item}/installed-metadata"))
        get_filename_component(PACKAGE_NAME "${item}" NAME)
        message(STATUS "ndk-pkg package: ${PACKAGE_NAME}")

        if (IS_DIRECTORY            "${item}/${ANDROID_ABI}/lib-no-versioning/cmake/${PACKAGE_NAME}")
            set(${PACKAGE_NAME}_DIR "${item}/${ANDROID_ABI}/lib-no-versioning/cmake/${PACKAGE_NAME}")
        endif()
    endif()
endforeach()
