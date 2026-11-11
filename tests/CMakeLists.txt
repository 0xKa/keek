find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)

set(KEEK_TEST_HOME ${CMAKE_BINARY_DIR}/test-home)
set(KEEK_TEST_ENVIRONMENT
    QT_QPA_PLATFORM=offscreen:configfile=${CMAKE_CURRENT_SOURCE_DIR}/offscreen.json
    QT_QUICK_BACKEND=software
    QT_SCALE_FACTOR=1
    QT_FONT_DPI=96
    LC_ALL=C.UTF-8
    TZ=UTC
    KEEK_AUTOPLAY=0
    XDG_CONFIG_HOME=${KEEK_TEST_HOME}/config
    XDG_DATA_HOME=${KEEK_TEST_HOME}/data
    XDG_CACHE_HOME=${KEEK_TEST_HOME}/cache
    XDG_STATE_HOME=${KEEK_TEST_HOME}/state
)

function(keek_add_test name)
    qt_add_executable(tst_${name} tst_${name}.cpp)
    target_link_libraries(tst_${name} PRIVATE Qt6::Test ${ARGN})
    target_compile_definitions(tst_${name} PRIVATE
        KEEK_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
        KEEK_REFERENCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/reference"
        KEEK_TEST_OUTPUT_DIR="${CMAKE_BINARY_DIR}/test-output"
    )
endfunction()

keek_add_test(renderers keekcore keekcoreplugin)
add_test(NAME renderers COMMAND tst_renderers)
set_tests_properties(renderers PROPERTIES ENVIRONMENT "${KEEK_TEST_ENVIRONMENT}" LABELS unit)

keek_add_test(listing keekcore keekcoreplugin)
add_test(NAME listing COMMAND tst_listing)
set_tests_properties(listing PROPERTIES ENVIRONMENT "${KEEK_TEST_ENVIRONMENT}" LABELS unit)

keek_add_test(controller keekcore keekcoreplugin)
add_test(NAME controller COMMAND tst_controller)
set_tests_properties(controller PROPERTIES ENVIRONMENT "${KEEK_TEST_ENVIRONMENT}" LABELS offscreen)

keek_add_test(previews keekcore keekcoreplugin)
add_test(NAME previews COMMAND tst_previews)
set_tests_properties(previews PROPERTIES ENVIRONMENT "${KEEK_TEST_ENVIRONMENT}" LABELS offscreen)

keek_add_test(cli Qt6::Gui)
add_test(NAME cli COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:tst_cli>)
set_tests_properties(cli PROPERTIES
    ENVIRONMENT "${KEEK_TEST_ENVIRONMENT};KEEK_TEST_PRIVATE_BUS=1;KEEK_BIN=$<TARGET_FILE:keek>;KEEKD_BIN=$<TARGET_FILE:keekd>"
    LABELS session
)

add_test(NAME shortcut COMMAND ${DBUS_RUN_SESSION} -- ${CMAKE_CURRENT_SOURCE_DIR}/shortcut-test.sh)
set_tests_properties(shortcut PROPERTIES LABELS unit)

add_test(NAME dolphin COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/dolphin-session.sh)
set_tests_properties(dolphin PROPERTIES
    ENVIRONMENT "KEEK_BIN_DIR=$<TARGET_FILE_DIR:keek>;KEEK_SERVICE_MENU=${CMAKE_BINARY_DIR}/data/keek.desktop;KEEK_FIXTURES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
    SKIP_RETURN_CODE 77
    TIMEOUT 120
    LABELS dolphin
)
