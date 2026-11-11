qt_add_executable(keek main.cpp)

target_link_libraries(keek PRIVATE Qt6::Core Qt6::DBus keekconfig)

install(TARGETS keek ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
