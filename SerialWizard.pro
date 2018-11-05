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
    FrameInfoDialog.cpp \
    CalculateCheckSumDialog.cpp \
    data/AbstractReadWriter.cpp \
    data/BridgeReadWriter.cpp \
    data/SerialReadWriter.cpp \
    data/TcpReadWriter.cpp \
    serial/SerialController.cpp \
    serial/NormalSerialController.cpp \
    serial/LineSerialController.cpp \
    serial/FrameSerialController.cpp \
    serial/FixedBytesSerialController.cpp

HEADERS  += mainwindow.h \
    global.h \
    FrameInfoDialog.h \
    CalculateCheckSumDialog.h \
    data/AbstractReadWriter.h \
    data/BridgeReadWriter.h \
    data/SerialReadWriter.h \
    data/TcpReadWriter.h \
    serial/SerialController.h \
    serial/LineSerialController.h \
    serial/FrameSerialController.h \
    serial/FixedBytesSerialController.h \
    serial/NormalSerialController.h
