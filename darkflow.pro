#-------------------------------------------------
#
# Project created by QtCreator 2015-10-01T21:43:39
#
#-------------------------------------------------

QT       += core gui

*-g++* {
# If you get linker errors about undefined references to symbols that
# involve types in the std::__cxx11 namespace
#    QMAKE_CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
    QMAKE_CXXFLAGS += -fopenmp -Wall -Werror -D_REENTRANT
    QMAKE_CXXFLAGS_RELEASE += -O9 -march=native -mfpmath=sse
    QMAKE_CXXFLAGS_DEBUG += -ggdb3
    QMAKE_LFLAGS +=  -fopenmp
    QMAKE_CFLAGS += -fopenmp -Wall -Werror -D_REENTRANT
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_INCDIR += core operators algorithms scene ui

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
    core/operatorinput.cpp \
    core/operatoroutput.cpp \
    core/operatorparameter.cpp \
    core/operatorparameterdropdown.cpp \
    core/operatorparameterfilescollection.cpp \
    core/operatorworker.cpp \
    core/photo.cpp \
    ui/visualization.cpp \
    scene/process.cpp \
    ui/treephotoitem.cpp \
    ui/treeoutputitem.cpp \
    ui/slider.cpp \
    scene/processslider.cpp \
    core/operatorparameterslider.cpp \
    algorithms/exposure.cpp \
    algorithms/igamma.cpp \
    algorithms/lutbased.cpp \
    algorithms/algorithm.cpp \
    operators/opwhitebalance.cpp \
    algorithms/whitebalance.cpp \
    operators/opmodulate.cpp \
    operators/opigamma.cpp \
    algorithms/desaturateshadows.cpp \
    operators/opdesaturateshadows.cpp \
    algorithms/shapedynamicrange.cpp \
    algorithms/cielab.cpp \
    operators/opshapedynamicrange.cpp \
    operators/opsubtract.cpp \
    operators/opblackbody.cpp \
    operators/opflatfieldcorrection.cpp \
    operators/opintegration.cpp \
    operators/workerintegration.cpp \
    operators/opexposure.cpp \
    operators/opinvert.cpp \
    algorithms/invert.cpp \
    ui/tabletagsrow.cpp \
    ui/tablewidgetitem.cpp \
    operators/opcrop.cpp \
    ui/vispoint.cpp \
    operators/oploadvideo.cpp \
    operators/workerloadvideo.cpp \
    operators/opblend.cpp \
    operators/workerblend.cpp \
    ui/fullscreenview.cpp \
    operators/opmultiplexer.cpp \
    operators/opdemultiplexer.cpp \
    operators/oprgbdecompose.cpp \
    operators/oprgbcompose.cpp \
    operators/opequalize.cpp \
    algorithms/channelmixer.cpp \
    operators/opchannelmixer.cpp \
    algorithms/colorfilter.cpp \
    operators/opcolorfilter.cpp \
    operators/opmicrocontrasts.cpp \
    operators/opunsharpmask.cpp \
    operators/opgaussianblur.cpp \
    operators/opblur.cpp \
    operators/opthreshold.cpp \
    algorithms/threshold.cpp \
    operators/opdeconvolution.cpp \
    operators/workerdeconvolution.cpp \
    operators/opdebayer.cpp \
    operators/workerdebayer.cpp \
    operators/oploadimage.cpp \
    operators/workerloadimage.cpp \
    operators/opconvolution.cpp \
    operators/workerconvolution.cpp \
    operators/opexnihilo.cpp \
    operators/oploadraw.cpp \
    operators/oppassthrough.cpp \
    operators/oprotate.cpp \
    operators/workerloadraw.cpp \
    algorithms/bayer.c \
    algorithms/rawinfo.cpp \
    operators/opcmydecompose.cpp \
    operators/opcmycompose.cpp \
    operators/oproll.cpp \
    operators/opscale.cpp \
    operators/oplimereg.cpp \
    operators/workerlimereg.cpp \
    operators/opssdreg.cpp \
    operators/workerssdreg.cpp \
    operators/opbracketing.cpp \
    operators/opgradientevaluation.cpp \
    operators/workergradientevaluation.cpp \
    operators/oplevel.cpp \
    operators/oplevelpercentile.cpp \
    operators/opflip.cpp \
    operators/opflop.cpp \
    operators/openhance.cpp \
    operators/opdespeckle.cpp \
    operators/opnormalize.cpp \
    operators/opadaptivethreshold.cpp \
    operators/opreducenoise.cpp \
    algorithms/hotpixels.cpp \
    operators/ophotpixels.cpp \
    operators/opcolor.cpp

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
    core/operatorinput.h \
    core/operatoroutput.h \
    core/operatorparameter.h \
    core/operatorparameterdropdown.h \
    core/operatorparameterfilescollection.h \
    core/operatorworker.h \
    core/photo.h \
    ui/visualization.h \
    scene/process.h \
    ui/treephotoitem.h \
    ui/treeoutputitem.h \
    ui/slider.h \
    scene/processslider.h \
    core/operatorparameterslider.h \
    algorithms/exposure.h \
    algorithms/igamma.h \
    algorithms/lutbased.h \
    algorithms/algorithm.h \
    operators/opwhitebalance.h \
    algorithms/whitebalance.h \
    operators/opmodulate.h \
    operators/opigamma.h \
    algorithms/desaturateshadows.h \
    algorithms/cielab.h \
    operators/opdesaturateshadows.h \
    algorithms/shapedynamicrange.h \
    operators/opshapedynamicrange.h \
    operators/opsubtract.h \
    operators/opblackbody.h \
    operators/opflatfieldcorrection.h \
    operators/opintegration.h \
    operators/workerintegration.h \
    operators/opexposure.h \
    operators/opinvert.h \
    algorithms/invert.h \
    ui/tabletagsrow.h \
    ui/tablewidgetitem.h \
    operators/opcrop.h \
    ui/vispoint.h \
    operators/oploadvideo.h \
    operators/workerloadvideo.h \
    operators/opblend.h \
    operators/workerblend.h \
    ui/fullscreenview.h \
    operators/opmultiplexer.h \
    operators/opdemultiplexer.h \
    operators/oprgbdecompose.h \
    operators/oprgbcompose.h \
    operators/opequalize.h \
    algorithms/channelmixer.h \
    operators/opchannelmixer.h \
    algorithms/colorfilter.h \
    operators/opcolorfilter.h \
    operators/opmicrocontrasts.h \
    operators/opunsharpmask.h \
    operators/opgaussianblur.h \
    operators/opblur.h \
    operators/opthreshold.h \
    algorithms/threshold.h \
    operators/opdeconvolution.h \
    operators/workerdeconvolution.h \
    operators/opdebayer.h \
    operators/workerdebayer.h \
    operators/oploadimage.h \
    operators/workerloadimage.h \
    operators/opconvolution.h \
    operators/workerconvolution.h \
    operators/oploadraw.h \
    operators/opexnihilo.h \
    operators/oprotate.h \
    operators/oppassthrough.h \
    operators/workerloadraw.h \
    algorithms/rawinfo.h \
    algorithms/bayer.h \
    operators/opcmydecompose.h \
    operators/opcmycompose.h \
    operators/oproll.h \
    operators/opscale.h \
    operators/oplimereg.h \
    operators/workerlimereg.h \
    operators/opssdreg.h \
    operators/workerssdreg.h \
    operators/opbracketing.h \
    operators/opgradientevaluation.h \
    operators/workergradientevaluation.h \
    operators/oplevel.h \
    operators/oplevelpercentile.h \
    operators/opflip.h \
    operators/opflop.h \
    operators/openhance.h \
    operators/opdespeckle.h \
    operators/opnormalize.h \
    operators/opadaptivethreshold.h \
    operators/opreducenoise.h \
    algorithms/hotpixels.h \
    operators/ophotpixels.h \
    operators/opcolor.h

FORMS    += \
    ui/aboutdialog.ui \
    ui/filesselection.ui \
    ui/mainwindow.ui \
    ui/projectproperties.ui \
    ui/visualization.ui \
    ui/slider.ui \
    ui/fullscreenview.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += Magick++ libavformat libavcodec libavutil liblimereg
#unix: PKGCONFIG += GraphicsMagick++ libavformat libavcodec libavutil
