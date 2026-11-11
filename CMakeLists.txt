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
find_package(KF6 6.0 REQUIRED COMPONENTS WindowSystem)

qt_standard_project_setup(REQUIRES 6.8)

add_compile_options(-Wall -Wextra -Wpedantic)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
