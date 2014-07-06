QT       += core testlib
QT       -= gui

TARGET = tests
CONFIG   += console

TEMPLATE = app

INCLUDEPATH += ..
LIBPATH += ..
LIBS += -lqtfjsonrpc

SOURCES += main.cpp \
    test_nnamedservice.cpp

HEADERS += \
    test_nnamedservice.h

tests.extra = @./tests
tests.path = /tmp

INSTALLS += tests





