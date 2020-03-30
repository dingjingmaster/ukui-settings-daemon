TEMPLATE = lib
TARGET = media-keys

QT += dbus widgets
CONFIG += no_keywords c++11 plugin link_pkgconfig
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

include($$PWD/../../common/common.pri)

INCLUDEPATH += \
    $$PWD/../common/

PKGCONFIG += \
    gdk-3.0 \
    gobject-2.0 \
    dbus-glib-1

SOURCES += \
    $$PWD/mediakey-plugin.cpp \
    $$PWD/mediakey-manager.cpp \
    $$PWD/manager-interface.cpp \
    mediakey-window.cpp

HEADERS += \
    $$PWD/mediakey-plugin.h \
    $$PWD/mediakey-manager.h \
    $$PWD/manager-interface.h \
    mediakey-window.h

DESTDIR = $$PWD/

media_keys_lib.path = /usr/local/lib/ukui-settings-daemon/
media_keys_lib.files = $$PWD/libmedia-keys.so

INSTALLS += media_keys_lib
