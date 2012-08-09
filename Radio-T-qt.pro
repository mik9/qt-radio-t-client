#-------------------------------------------------
#
# Project created by QtCreator 2012-08-08T10:00:43
#
#-------------------------------------------------

QT       += core gui network xml

TARGET = Radio-T-qt
TEMPLATE = app


SOURCES += main.cpp\
        chatwidget.cpp \
    simplecrypt.cpp

HEADERS  += chatwidget.h \
    simplecrypt.h

FORMS    += chatwidget.ui

CONFIG(debug, debug|release) {
    windows: LIBS += -lqxmpp_d0
} else {
    windows: LIBS += -lqxmpp0
}
unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += qxmpp
