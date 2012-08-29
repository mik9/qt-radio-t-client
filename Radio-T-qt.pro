#-------------------------------------------------
#
# Project created by QtCreator 2012-08-08T10:00:43
#
#-------------------------------------------------

QT       += core gui network xml phonon opengl

TARGET = Radio-T-qt
TEMPLATE = app

windows {
    LIBS += -ldnsapi -lws2_32
}
static {
    # FIXME: g++ wont link this libraries unless we manually move it to the end in Makefile...
    windows {
        LIBS += -ldxguid -lstrmiids -lmsdmo -ldmoguids
    }
    QTPLUGIN += phonon_ds9
    DEFINES += STATIC
    message("Static build.")
}

SOURCES += main.cpp\
        chatwidget.cpp \
    simplecrypt.cpp \
    twitterwidget.cpp \
    playerwidget.cpp

HEADERS  += chatwidget.h \
    simplecrypt.h \
    key.h \
    twitterwidget.h \
    playerwidget.h

FORMS    += chatwidget.ui \
    playerwidget.ui

INCLUDEPATH += qtweetlib base client

include(qtweetlib/qtweetlib.pro)
include(base/base.pro)
include(client/client.pro)
