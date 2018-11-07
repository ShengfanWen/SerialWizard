#-------------------------------------------------
#
# Project created by QtCreator 2017-08-03T09:01:26
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
                                QT += serialport
                                QT += network

CONFIG += c++11

TARGET = SerialWizard
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    global.cpp \
    data/SerialReadWriter.cpp \
    serial/SerialController.cpp \
    serial/NormalSerialController.cpp

HEADERS  += mainwindow.h \
    global.h \
    data/SerialReadWriter.h \
    serial/SerialController.h \
    serial/NormalSerialController.h
