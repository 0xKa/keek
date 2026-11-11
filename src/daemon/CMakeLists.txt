qt_add_executable(keekd
    dbusservice.cpp
    dbusservice.h
    jsonlog.cpp
    jsonlog.h
    main.cpp
)

target_link_libraries(keekd PRIVATE keekcore keekcoreplugin Qt6::DBus)

install(TARGETS keekd ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
