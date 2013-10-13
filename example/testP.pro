#-------------------------------------------------
#
# Project created by QtCreator 2013-01-23T16:21:12
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = testP
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += /usr/include

SOURCES += main.cpp \
    fastcgi.cpp \
    nnamedservice.cpp \
    testservice.cpp

HEADERS += \
    fastcgi.h \
    nnamedservice.h \
    testservice.h

unix|win32: LIBS += -lfcgi

OTHER_FILES += \
    TODO.txt
