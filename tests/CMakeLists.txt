find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)

qt_add_executable(tst_controller tst_controller.cpp)
target_link_libraries(tst_controller PRIVATE keekcore keekcoreplugin Qt6::Test)
add_test(NAME controller COMMAND tst_controller)
set_tests_properties(controller PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    LABELS offscreen
)

qt_add_executable(tst_cli tst_cli.cpp)
target_link_libraries(tst_cli PRIVATE Qt6::Test Qt6::Gui)
add_test(NAME cli COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:tst_cli>)
set_tests_properties(cli PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;KEEK_TEST_PRIVATE_BUS=1;KEEK_BIN=$<TARGET_FILE:keek>;KEEKD_BIN=$<TARGET_FILE:keekd>"
    LABELS session
)
