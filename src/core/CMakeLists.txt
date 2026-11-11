qt_add_library(keekcore STATIC)

set_source_files_properties(Metrics.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(keekcore
    URI Keek
    VERSION 1.0
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Keek
    QML_FILES
        CenteredMessage.qml
        FallbackView.qml
        FileSummary.qml
        FolderView.qml
        ImageView.qml
        Metrics.qml
        Popup.qml
        PreviewView.qml
        TextView.qml
    SOURCES
        controller.cpp
        controller.h
        fallbackrenderer.cpp
        filedetails.cpp
        filedetails.h
        fileicon.cpp
        fileicon.h
        folderrenderer.cpp
        folderinfo.cpp
        folderinfo.h
        folderlisting.cpp
        folderlisting.h
        imageinfo.cpp
        imageinfo.h
        imagerenderer.cpp
        logging.cpp
        logging.h
        popupwindow.cpp
        popupwindow.h
        renderer.h
        rendererregistry.cpp
        rendererregistry.h
        textfile.cpp
        textfile.h
        textrenderer.cpp
        thumbnail.cpp
        thumbnail.h
)

target_include_directories(keekcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(keekcore PUBLIC Qt6::Quick KF6::WindowSystem KF6::KIOGui KF6::CoreAddons KF6::ConfigCore Qt6::Concurrent keekconfig)
