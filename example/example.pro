QT       += core
QT       -= gui

TARGET = qtfjsonrpc-example
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

example.files = example.pro  main.cpp  testservice.cpp  testservice.h
example.path = /usr/share/doc/qtfjsonrpc-dev/example

INSTALLS += example
