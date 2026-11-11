qt_add_library(keekcore STATIC)

qt_add_qml_module(keekcore
    URI Keek
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Keek
    QML_FILES
        Popup.qml
    SOURCES
        controller.cpp
        controller.h
        logging.cpp
        logging.h
        popupwindow.cpp
        popupwindow.h
)

target_include_directories(keekcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(keekcore PUBLIC Qt6::Quick KF6::WindowSystem keekconfig)
