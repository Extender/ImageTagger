#-------------------------------------------------
#
# Project created by QtCreator 2016-07-30T12:21:44
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webenginewidgets

TARGET = ImageTagger
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    text.cpp \
    graphicssceneex.cpp \
    graphicsviewex.cpp \
    io.cpp \
    mainwindowex.cpp \
    webbrowserdialog.cpp \
    webenginepageex.cpp

HEADERS  += mainwindow.h \
    text.h \
    graphicssceneex.h \
    graphicsviewex.h \
    io.h \
    mainwindowex.h \
    defaulttagcolor.h \
    extcolordefs.h \
    webbrowserdialog.h \
    webenginepageex.h

FORMS    += mainwindow.ui \
    webbrowserdialog.ui

RESOURCES += \
    resources.qrc
