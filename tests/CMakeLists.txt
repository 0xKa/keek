qt_add_executable(tst_controller tst_controller.cpp)
target_link_libraries(tst_controller PRIVATE keekcore keekcoreplugin Qt6::Test)
add_test(NAME controller COMMAND tst_controller)
set_tests_properties(controller PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    LABELS offscreen
)
