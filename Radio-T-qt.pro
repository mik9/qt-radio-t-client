#-------------------------------------------------
#
# Project created by QtCreator 2012-08-08T10:00:43
#
#-------------------------------------------------

QT       += core gui

TARGET = Radio-T-qt
TEMPLATE = app


SOURCES += main.cpp\
        chatwidget.cpp \
    simplecrypt.cpp

HEADERS  += chatwidget.h \
    simplecrypt.h

FORMS    += chatwidget.ui

debug:LIBS += -lqxmpp_d0
#release:LIBS += -lqxmpp0
