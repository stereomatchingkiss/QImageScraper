#-------------------------------------------------
#
# Project created by QtCreator 2017-05-16T10:41:54
#
#-------------------------------------------------

QT       += core gui webenginewidgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QImageScraper
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += ..
INCLUDEPATH += ../3rdLibs/qslog
#Without this include path, Qt cannot find web_view.hpp
INCLUDEPATH += ui

include(../3rdLibs/qslog/QsLog.pri)

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32{

RC_FILE = icons/QImageScraper.rc
TARGET_SRC  = $${OUT_PWD}/release/$${TARGET}
TARGET_DEST = $${PWD}/package/windows_installer/packages/main.component/data/$${TARGET}

}

macx{

ICON = icons/QImageScraper.ico.icns
TARGET_SRC  = $${OUT_PWD}/$${TARGET}.app
TARGET_DEST = $${PWD}/package/mac_package/$${TARGET}.app
QMAKE_POST_LINK += $$quote(cp -R $${TARGET_SRC} $${TARGET_DEST}$$escape_expand(\n\t))

}

linux-g++{
    TARGET_SRC  = $${OUT_PWD}/$${TARGET}
    TARGET_DEST = $${PWD}/package/linux_appimage/$${TARGET}
    if( equals(TEMPLATE, app) || equals(TEMPLATE, vcapp) ){
        # nothing to do here
    }
    if( equals(TEMPLATE, lib) || equals(TEMPLATE, vclib) ){
        TARGET_SRC   = $${TARGET_SRC}.so
        TARGET_DEST  = $${TARGET_DEST}.so
    }
    QMAKE_POST_LINK += $$quote(cp $${TARGET_SRC} $${TARGET_DEST}$$escape_expand(\n\t))
}

win32 {
    if( equals(TEMPLATE, app) || equals(TEMPLATE, vcapp) ){
        TARGET_SRC   = $${TARGET_SRC}.exe
        TARGET_DEST  = $${TARGET_DEST}.exe
    }
    if( equals(TEMPLATE, lib) || equals(TEMPLATE, vclib) ){
        TARGET_SRC   = $${TARGET_SRC}.dll
        TARGET_DEST  = $${TARGET_DEST}.dll
    }
    TARGET_SRC  ~= s,/,\\,g # fix slashes
    TARGET_DEST ~= s,/,\\,g # fix slashes
    QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${TARGET_SRC} $${TARGET_DEST}$$escape_expand(\n\t))
}

message("Will copy $${TARGET_SRC} to $${TARGET_DEST}")

SOURCES += main.cpp\
        mainwindow.cpp \
    core/bing_image_search.cpp \
    core/image_search.cpp \
    core/js_function.cpp \    
    ui/web_view.cpp \
    ../qt_enhance/network/download_supervisor.cpp \
    core/utility.cpp \
    ui/general_settings.cpp \
    core/global_constant.cpp \
    ui/info_dialog.cpp \
    core/google_image_search.cpp \
    core/yahoo_image_search.cpp \
    ../qt_enhance/utility/qte_utility.cpp \
    core/image_downloader.cpp \
    ui/proxy_settings.cpp \
    ui/settings_manager.cpp \
    ui/proxy_delegate.cpp

HEADERS  += mainwindow.hpp \
    core/bing_image_search.hpp \
    core/image_search.hpp \
    core/js_function.hpp \    
    ui/web_view.hpp \
    ../qt_enhance/network/download_supervisor.hpp \
    core/utility.hpp \
    ui/general_settings.hpp \
    core/global_constant.hpp \
    ui/info_dialog.hpp \
    core/google_image_search.hpp \
    core/image_search_error.hpp \
    core/yahoo_image_search.hpp \
    ../qt_enhance/utility/qte_utility.hpp \
    core/image_downloader.hpp \
    ui/proxy_settings.hpp \
    ui/settings_manager.hpp \
    ui/proxy_delegate.hpp

FORMS    += mainwindow.ui \
    ui/general_settings.ui \
    ui/info_dialog.ui \
    ui/proxy_settings.ui \
    ui/settings_manager.ui

RESOURCES += \
    icons.qrc

DISTFILES +=
