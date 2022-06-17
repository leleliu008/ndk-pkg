cmake_minimum_required(VERSION 3.0)

project(ndk-pkg C)

set(PROJECT_VERSION       1.0.0)
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

########################################################################################

aux_source_directory(. SRCS)

add_executable(ndk-pkg "${SRCS}")

########################################################################################

include(GNUInstallDirs)

install(TARGETS ndk-pkg RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(
    FILES       "${CMAKE_CURRENT_SOURCE_DIR}/../../zsh-completion/_ndk-pkg"
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/zsh/site-functions"
)
