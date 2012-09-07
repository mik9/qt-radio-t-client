#-------------------------------------------------
#
# Project created by QtCreator 2012-08-08T10:00:43
#
#-------------------------------------------------

QT       += core gui network xml

TARGET = Radio-T-qt
TEMPLATE = app

win32:LIBS += -lmpg123-0 -lao-4
else:LIBS += -lmpg123 -lao
#LIBS += -lfftreal

SOURCES += main.cpp\
        chatwidget.cpp \
    simplecrypt.cpp \
    twitterwidget.cpp \
    playerwidget.cpp \
#    spectrumanalyser.cpp \
#    frequencyspectrum.cpp \
#    utils.cpp \
    spectrumwidget.cpp

HEADERS  += chatwidget.h \
    simplecrypt.h \
    key.h \
    twitterwidget.h \
    playerwidget.h \
#    spectrumanalyser.h \
#    frequencyspectrum.h \
#    utils.h \
#    spectrum.h \
    sleeper.h \
    spectrumwidget.h \

FORMS    += chatwidget.ui \
    playerwidget.ui

INCLUDEPATH += qtweetlib base client

include(qtweetlib/qtweetlib.pro)
include(base/base.pro)
include(client/client.pro)
