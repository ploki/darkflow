#-------------------------------------------------
#
# Project created by QtCreator 2015-10-01T21:43:39
#
#-------------------------------------------------

QT       += core gui

*-g++* {
    QMAKE_CXXFLAGS += -fopenmp -Wall -Werror
    QMAKE_CXXFLAGS_RELEASE += -O9
    QMAKE_CXXFLAGS_DEBUG += -ggdb3
    QMAKE_LFLAGS +=  -fopenmp
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_INCDIR += core operators manipulation scene ui

TARGET = darkflow
TEMPLATE = app


SOURCES +=\
    ui/aboutdialog.cpp \
    ui/filesselection.cpp \
    ui/mainwindow.cpp \
    scene/processbutton.cpp \
    scene/processconnection.cpp \
    scene/processdropdown.cpp \
    scene/processfilescollection.cpp \
    scene/processnode.cpp \
    scene/processport.cpp \
    scene/processprogressbar.cpp \
    scene/processscene.cpp \
    ui/projectproperties.cpp \
    ui/main.cpp \
    core/operator.cpp \
    operators/operatorexnihilo.cpp \
    core/operatorinput.cpp \
    operators/operatorloadraw.cpp \
    core/operatoroutput.cpp \
    core/operatorparameter.cpp \
    core/operatorparameterdropdown.cpp \
    core/operatorparameterfilescollection.cpp \
    operators/operatorpassthrough.cpp \
    core/operatorworker.cpp \
    core/photo.cpp \
    operators/rawconvert.cpp \
    operators/rawinfo.cpp \
    ui/visualization.cpp \
    scene/process.cpp \
    ui/treephotoitem.cpp \
    ui/treeoutputitem.cpp \
    manipulation/igamma.cpp \
    manipulation/manipulation.cpp \
    manipulation/exposure.cpp \
    manipulation/lutbased.cpp

HEADERS  += \
    ui/aboutdialog.h \
    ui/filesselection.h \
    ui/mainwindow.h \
    scene/processbutton.h \
    scene/processconnection.h \
    scene/processdropdown.h \
    scene/processnode.h \
    scene/processport.h \
    scene/processfilescollection.h \
    scene/processprogressbar.h \
    scene/processscene.h \
    ui/projectproperties.h \
    core/operator.h \
    operators/operatorexnihilo.h \
    core/operatorinput.h \
    core/operatoroutput.h \
    core/operatorparameter.h \
    core/operatorparameterdropdown.h \
    core/operatorparameterfilescollection.h \
    operators/operatorpassthrough.h \
    operators/operatorloadraw.h \
    core/operatorworker.h \
    core/photo.h \
    operators/rawconvert.h \
    operators/rawinfo.h \
    ui/visualization.h \
    scene/process.h \
    ui/treephotoitem.h \
    ui/treeoutputitem.h \
    manipulation/igamma.h \
    manipulation/manipulation.h \
    manipulation/exposure.h \
    manipulation/lutbased.h

FORMS    += \
    ui/aboutdialog.ui \
    ui/filesselection.ui \
    ui/mainwindow.ui \
    ui/projectproperties.ui \
    ui/visualization.ui \
    ui/slider.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += Magick++
