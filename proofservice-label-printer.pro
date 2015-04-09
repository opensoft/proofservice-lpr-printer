TARGET = proofservice-label-printer
TEMPLATE = app

VERSION = 0.14.11.6

QT += core network
QT -= gui

CONFIG += c++11 proof proofcore proofnetwork proofhardwarelabelprinter
CONFIG -= app_bundle

SOURCES += \
    main.cpp \
    labelprinterrestserver.cpp \
    labelprinterhandler.cpp

HEADERS += \
    proofservice_label_printer_global.h \
    labelprinterrestserver.h \
    labelprinterhandler.h
