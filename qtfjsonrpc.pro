QT       += core
QT       -= gui

TARGET = qtfjsonrpc
CONFIG   += console dll
VERSION = 0.1.0

TEMPLATE = lib

INCLUDEPATH += /usr/include

SOURCES += nfastcgi.cpp \
    nnamedservice.cpp

HEADERS += \
    nfastcgi.h \
    nnamedservice.h

LIBS += -lfcgi

lib.files = libqtfjsonrpc.so.0.1.0
lib.extra = ln -s libqtfjsonrpc.so.0.1.0 $(INSTALL_ROOT)/usr/lib/libqtfjsonrpc.so.0 ;
lib.extra += ln -s libqtfjsonrpc.so.0.1.0 $(INSTALL_ROOT)/usr/lib/libqtfjsonrpc.so.0.1
lib.path = /usr/lib

devellib.files = libqtfjsonrpc.so
devellib.extra = ln -s libqtfjsonrpc.so.0.1.0 $(INSTALL_ROOT)/usr/lib/libqtfjsonrpc.so
devellib.path = /usr/lib
develheaders.files = $$HEADERS
develheaders.path = /usr/include

INSTALLS += lib devellib develheaders



