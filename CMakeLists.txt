cmake_minimum_required(VERSION 3.25)

project(keek VERSION 0.1.0 LANGUAGES CXX)

set(KEEK_APP_ID "io.github._0xKa.keek")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ECM 6.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui DBus Qml Quick Test)
find_package(KF6 6.0 REQUIRED COMPONENTS WindowSystem KIO CoreAddons Config)

qt_standard_project_setup(REQUIRES 6.8)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(src)
add_subdirectory(data)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

include(KDEClangFormat)
file(GLOB_RECURSE KEEK_CXX_SOURCES src/*.cpp src/*.h tests/*.cpp tests/*.h)
kde_clang_format(${KEEK_CXX_SOURCES})

add_custom_target(screenshot-page
    COMMAND ${CMAKE_COMMAND} -E env KEEK_BIN_DIR=$<TARGET_FILE_DIR:keek>
        ${CMAKE_SOURCE_DIR}/scripts/screenshot-page.sh ${CMAKE_BINARY_DIR}/screenshots
    DEPENDS keek keekd
    USES_TERMINAL
)
