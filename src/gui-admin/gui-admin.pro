#-------------------------------------------------
#
# Project created by QtCreator 2019-11-23T00:24:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui-admin
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
        mainwindow.cpp \
    manager.cpp \
    setpwd.cpp \
    setmaincontrol.cpp \
    addcharacter.cpp \
    removecharacter.cpp \
    setdata.cpp \
    setposition.cpp \
    vlogin.cpp

HEADERS += \
        mainwindow.h \
    manager.h \
    setpwd.h \
    setmaincontrol.h \
    addcharacter.h \
    removecharacter.h \
    setdata.h \
    setposition.h \
    vlogin.h

FORMS += \
        mainwindow.ui \
    manager.ui \
    setpwd.ui \
    setmaincontrol.ui \
    addcharacter.ui \
    removecharacter.ui \
    setdata.ui \
    setposition.ui \
    vlogin.ui

LIBS += -luuid -lleveldb -lpthread -lRakNetDLL -llua -ldl
