QT       += core
QT       -= gui

TARGET = qtfjsonrpc
CONFIG   += console dll
VERSION = 0.1.0

TEMPLATE = lib

INCLUDEPATH += /usr/include

SOURCES += fastcgi.cpp \
    nnamedservice.cpp

HEADERS += \
    fastcgi.h \
    nnamedservice.h

LIBS += -lfcgi
