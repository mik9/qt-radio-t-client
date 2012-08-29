#-------------------------------------------------
#
# Project created by QtCreator 2012-08-08T10:00:43
#
#-------------------------------------------------

QT       += core gui network xml phonon

TARGET = Radio-T-qt
TEMPLATE = app


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
LIBS += -ldnsapi -lws2_32
