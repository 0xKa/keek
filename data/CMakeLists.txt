configure_file(keek.desktop.in keek.desktop @ONLY)
configure_file(dbus.service.in ${KEEK_APP_ID}.service @ONLY)
configure_file(keekd.desktop.in keekd.desktop @ONLY)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/keek.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kio/servicemenus)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${KEEK_APP_ID}.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/keekd.desktop DESTINATION ${KDE_INSTALL_DATADIR}/keek)
