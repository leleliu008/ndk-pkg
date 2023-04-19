# https://developer.android.com/ndk/guides/cmake

if (NOT NDKPKG_HOME)
    set(NDKPKG_HOME "$ENV{HOME}/.ndk-pkg")
endif()

message(STATUS "NDKPKG_HOME=${NDKPKG_HOME}")
message(STATUS "NDKPKG_ROOT=${NDKPKG_HOME}/installed/android-${ANDROID_PLATFORM_LEVEL}")

if (NOT IS_DIRECTORY "${NDKPKG_HOME}/installed/android-${ANDROID_PLATFORM_LEVEL}")
    message(FATAL_ERROR "no any ndk-pkg packages installed.")
endif()

file(GLOB items LIST_DIRECTORIES true "${NDKPKG_HOME}/installed/android-${ANDROID_PLATFORM_LEVEL}/${ANDROID_ABI}/*")
foreach(item ${items})
    if ((IS_DIRECTORY ${item}) AND (EXISTS "${item}/.ndk-pkg/receipt.yml") AND (EXISTS "${item}/.ndk-pkg/manifest.txt"))
        get_filename_component(PACKAGE_NAME "${item}" NAME)
        message(STATUS "ndk-pkg package: ${PACKAGE_NAME}")

        if (IS_DIRECTORY            "${item}/lib-no-versioning/cmake/${PACKAGE_NAME}")
            set(${PACKAGE_NAME}_DIR "${item}/lib-no-versioning/cmake/${PACKAGE_NAME}")
        endif()
    endif()
endforeach()
