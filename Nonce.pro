#-------------------------------------------------
#
# Project created by QtCreator 2018-05-31T17:10:28
#
#-------------------------------------------------

QT       += core gui 

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Nonce
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
           mainwindow.cpp \
           sha2.cpp \
           block_hash.cpp

HEADERS  += mainwindow.h \
            sha2.h \
            block_hash.h \
            utils.hpp

FORMS    += mainwindow.ui
