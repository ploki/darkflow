#-------------------------------------------------
#
# Project created by QtCreator 2015-10-01T21:43:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = darkflow
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    aboutdialog.cpp \
    projectproperties.cpp \
    process.cpp \
    processscene.cpp \
    processnode.cpp \
    operator.cpp \
    operatorparameter.cpp \
    operatorinput.cpp \
    image.cpp \
    operatorexnihilo.cpp \
    operatorpaththrough.cpp \
    processbutton.cpp \
    processport.cpp \
    processconnection.cpp \
    filesselection.cpp \
    operatorparameterfilescollection.cpp \
    operatorparameterdropdown.cpp \
    operatorloadraw.cpp

HEADERS  += \
    mainwindow.h \
    aboutdialog.h \
    projectproperties.h \
    process.h \
    processscene.h \
    processnode.h \
    operator.h \
    operatorparameter.h \
    operatorinput.h \
    image.h \
    operatorexnihilo.h \
    operatorpaththrough.h \
    processbutton.h \
    processport.h \
    processconnection.h \
    filesselection.h \
    operatorparameterfilescollection.h \
    operatorparameterdropdown.h \
    operatorloadraw.h

FORMS    += mainwindow.ui \
    projectproperties.ui \
    aboutdialog.ui \
    filesselection.ui
