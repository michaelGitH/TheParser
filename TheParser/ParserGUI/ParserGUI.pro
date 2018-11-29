#-------------------------------------------------
#
# Project created by QtCreator 2018-07-27T02:58:04
#
#-------------------------------------------------

#QT       += core gui

CONFIG += c++11 #console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ParserGUI
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        parsergui.cpp \
    consolemode.cpp \
    parser.cpp

HEADERS += \
        parsergui.h \
    consolemode.h \
    parser.h

FORMS += \
        parsergui.ui

INCLUDEPATH += D:\Data\Documents\QtProjects\curl-7.59.0\include

DEFINES += CURL_STATICLIB

LIBS += -LD:\Data\Documents\QtProjects\curl-7.59.0\lib -lcurldll -lcurl -lwldap32 -lssl -lcrypto -lws2_32 -lgdi32 -liconv

INCLUDEPATH += D:\msys\home\Mihail\MsysProjects\icu\source\common

LIBS += -LD:\msys\home\Mihail\MsysProjects\icu\source\lib -licuuc
