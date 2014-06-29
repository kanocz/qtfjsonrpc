QT       += core
QT       -= gui

TARGET = testP
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ..
LIBPATH += ..
LIBS += -lqtfjsonrpc

SOURCES += main.cpp \
    testservice.cpp

HEADERS += \
    testservice.h
