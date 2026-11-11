qt_add_library(keekcore STATIC)

qt_add_qml_module(keekcore
    URI Keek
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Keek
    QML_FILES
        CenteredMessage.qml
        ImageView.qml
        PlaceholderView.qml
        Popup.qml
        PreviewView.qml
        TextView.qml
    SOURCES
        controller.cpp
        controller.h
        imageinfo.cpp
        imageinfo.h
        imagerenderer.cpp
        logging.cpp
        logging.h
        placeholderrenderer.cpp
        popupwindow.cpp
        popupwindow.h
        renderer.h
        rendererregistry.cpp
        rendererregistry.h
        textfile.cpp
        textfile.h
        textrenderer.cpp
)

target_include_directories(keekcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(keekcore PUBLIC Qt6::Quick KF6::WindowSystem keekconfig)
