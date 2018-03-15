#-------------------------------------------------
#
# Project created by QtCreator 2017-09-29T10:19:15
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = protocol_generator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    protocolfilereader.cpp \
    cppgenerator.cpp \
    luagenerator.cpp

HEADERS  += mainwindow.h \
    protocolfilereader.h \
    cppgenerator.h \
    luagenerator.h \
    common_tools.hpp


FORMS    += mainwindow.ui
