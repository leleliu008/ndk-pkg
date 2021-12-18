message(STATUS "CMAKE_HOST_SYSTEM_NAME=${CMAKE_HOST_SYSTEM_NAME}")

if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    message(FATAL_ERROR "ndk-pkg not support windows")
endif()

find_program(SHELL NAMES zsh bash dash ash sh REQUIRED)
message(STATUS "SHELL=${SHELL}")

############################################################

find_program(NDK_PKG_EXECUTABLE ndk-pkg REQUIRED)
message(STATUS "NDK_PKG_EXECUTABLE=${NDK_PKG_EXECUTABLE}")

############################################################

execute_process(
    COMMAND ${SHELL} ${NDK_PKG_EXECUTABLE} --version
    OUTPUT_VARIABLE NDK_PKG_VERSION
    RESULT_VARIABLE NDK_PKG_ERROR_CODE
    ERROR_VARIABLE  NDK_PKG_ERROR_MESSAGE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (${NDK_PKG_ERROR_CODE} STREQUAL 0)
    message(STATUS "NDK_PKG_VERSION=${NDK_PKG_VERSION}")
else()
    message(FATAL_ERROR "${NDK_PKG_ERROR_MESSAGE}")
endif()

unset(NDK_PKG_ERROR_CODE)
unset(NDK_PKG_ERROR_MESSAGE)

string(REPLACE "." ";" NDK_PKG_VERSION_PARTS ${NDK_PKG_VERSION})
list(GET NDK_PKG_VERSION_PARTS 0 NDK_PKG_VERSION_MAJOR)
list(GET NDK_PKG_VERSION_PARTS 1 NDK_PKG_VERSION_MINOR)
list(GET NDK_PKG_VERSION_PARTS 2 NDK_PKG_VERSION_PATCH)
message(STATUS "NDK_PKG_VERSION_MAJOR=${NDK_PKG_VERSION_MAJOR}")
message(STATUS "NDK_PKG_VERSION_MINOR=${NDK_PKG_VERSION_MINOR}")
message(STATUS "NDK_PKG_VERSION_PATCH=${NDK_PKG_VERSION_PATCH}")

############################################################

execute_process(
    COMMAND ${SHELL} ${NDK_PKG_EXECUTABLE} --homedir
    OUTPUT_VARIABLE NDK_PKG_HOME
    RESULT_VARIABLE NDK_PKG_ERROR_CODE
    ERROR_VARIABLE  NDK_PKG_ERROR_MESSAGE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (${NDK_PKG_ERROR_CODE} STREQUAL 0)
    message(STATUS "NDK_PKG_HOME=${NDK_PKG_HOME}")
else()
    message(FATAL_ERROR "${NDK_PKG_ERROR_MESSAGE}")
endif()

unset(NDK_PKG_ERROR_CODE)
unset(NDK_PKG_ERROR_MESSAGE)

############################################################

execute_process(
    COMMAND ${SHELL} ${NDK_PKG_EXECUTABLE} ls-available
    OUTPUT_VARIABLE NDK_PKG_AVAILABLE_PACKAGES
    RESULT_VARIABLE NDK_PKG_ERROR_CODE
    ERROR_VARIABLE  NDK_PKG_ERROR_MESSAGE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (${NDK_PKG_ERROR_CODE} STREQUAL 0)
    string(REPLACE "\n" ";" NDK_PKG_AVAILABLE_PACKAGES ${NDK_PKG_AVAILABLE_PACKAGES})
    #message(STATUS "NDK_PKG_AVAILABLE_PACKAGES=${NDK_PKG_AVAILABLE_PACKAGES}")
else()
    message(FATAL_ERROR "${NDK_PKG_ERROR_MESSAGE}")
endif()

############################################################

foreach(PKG ${NDK_PKG_AVAILABLE_PACKAGES})
    #message(STATUS "${PKG}")
    list(APPEND CMAKE_FIND_ROOT_PATH "${NDK_PKG_HOME}/install.d/${PKG}/${ANDROID_ABI}")
    if (EXISTS "${NDK_PKG_HOME}/install.d/${PKG}/${ANDROID_ABI}/bin/")
        list(APPEND CMAKE_IGNORE_PATH "${NDK_PKG_HOME}/install.d/${PKG}/${ANDROID_ABI}/bin")
    endif()
    if (EXISTS "${NDK_PKG_HOME}/install.d/${PKG}/${ANDROID_ABI}/sbin/")
        list(APPEND CMAKE_IGNORE_PATH "${NDK_PKG_HOME}/install.d/${PKG}/${ANDROID_ABI}/sbin")
    endif()
    if (EXISTS "${NDK_PKG_HOME}/install.d/${PKG}/${ANDROID_ABI}/lib/pkgconfig")
        set(PKG_CONFIG_LIBDIR "${NDK_PKG_HOME}/install.d/${PKG}/${ANDROID_ABI}/lib/pkgconfig:${PKG_CONFIG_LIBDIR}")
    endif()
    unset(PKG)
endforeach()

set(ENV{PKG_CONFIG_LIBDIR} "${PKG_CONFIG_LIBDIR}")

#message(STATUS "CMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}")
#message(STATUS "CMAKE_IGNORE_PATH=${CMAKE_IGNORE_PATH}")
#message(STATUS "PKG_CONFIG_LIBDIR=$ENV{PKG_CONFIG_LIBDIR}")
