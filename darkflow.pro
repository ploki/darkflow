#-------------------------------------------------
#
# Project created by QtCreator 2015-10-01T21:43:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#openmp support in clang doesn't seem to be as good as gcc
osx_openmp = 0
force_gcd = 0

contains(force_gcd, 1) {
    QMAKE_CFLAGS += -fblocks -DDFL_USE_GCD=1
    QMAKE_CXXFLAGS += -fblocks -DDFL_USE_GCD=1
    QMAKE_LFLAGS += -fblocks -ldispatch -lBlocksRuntime
    *-g++* {
        error("Grand Central Dispatch not supported by g++")
    }
}

contains(osx_openmp, 1) {
    #qtcreator refuse to use the compiler kit I want...
    QMAKE_LIBDIR = /usr/local/lib
    QMAKE_LINK = clang++-3.8
    QMAKE_CXX = "clang++-3.8 -stdlib=libc++"
    QMAKE_CC = clang-3.8
}

unix {
*-g++* | *clang* {
    !macx {
        *-g++-64 | linux-clang {
            message("x64 build")
        } else {
            message("x86 build")
            QMAKE_CFLAGS += -m32
            QMAKE_CXXFLAGS += -m32
            QMAKE_LFLAGS += -m32
            QMAKE_CXXFLAGS_RELEASE += -msse2 -mfpmath=sse
        }
    }
# If you get linker errors about undefined references to symbols that
# involve types in the std::__cxx11 namespace
#    QMAKE_CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=0
    QMAKE_CXXFLAGS += -std=c++11 -Wall -D_REENTRANT
    QMAKE_CXXFLAGS_RELEASE += -O2
    QMAKE_CXXFLAGS_DEBUG += -ggdb3 -ftrapv
    QMAKE_CFLAGS += -Wall -D_REENTRANT
    QMAKE_CXXFLAGS += -Werror -Wno-deprecated-declarations
    QMAKE_CFLAGS += -Werror -Wno-deprecated-declarations

    !macx | contains(osx_openmp, 1) {
        QMAKE_CXXFLAGS += -fopenmp
        QMAKE_CFLAGS += -fopenmp
        QMAKE_LFLAGS +=  -fopenmp
    }
    macx {
        QMAKE_CFLAGS += -gdwarf-2
        QMAKE_CXXFLAGS += -gdwarf-2
        QMAKE_LFLAGS += -rpath @executable_path/../Frameworks -rpath @executable_path/../Library
    }
}
}

!macx {
    ICON = icons/darkflow.png \
        icons/darkflow-256x256.ico \
        icons/darkflow-128x128.ico \
        icons/darkflow-96x96.ico \
        icons/darkflow-64x64.ico \
        icons/darkflow-48x48.ico \
        icons/darkflow-32x32.ico \
        icons/darkflow-24x24.ico
} else {
    ICON = icons/darkflow.icns
    QMAKE_INFO_PLIST = setup/darkflow.plist
}

unix {
    macx {
        message("OSX build")
        QT_CONFIG -= no-pkg-config
    }
    QMAKE_CXXFLAGS += -DHAVE_FFMPEG
    QMAKE_CFLAGS += -DHAVE_FFMPEG
    CONFIG += link_pkgconfig
    PKGCONFIG += Magick++ libavformat libavcodec libavutil fftw3
    #PKGCONFIG += GraphicsMagick++ libavformat libavcodec libavutil
    LIBS += -lfftw3_threads
}

win32 {
    QMAKE_CXXFLAGS += /wd4351 /wd4251 /wd4267 /openmp /MP /DHAVE_FFMPEG
    QMAKE_CFLAGS += /wd4351 /wd4251 /wd4267 /openmp /MP /DHAVE_FFMPEG
    QMAKE_LFLAGS += /LARGEADDRESSAWARE
    contains(QMAKE_TARGET.arch, x86_64) {
        message("x64 build")
        QMAKE_CXXFLAGS += -IC:\ImageMagick\6.9.3-Q16\include
        QMAKE_CFLAGS += -IC:\ImageMagick\6.9.3-Q16\include
        LIBS += -LC:\ImageMagick\6.9.3-Q16\lib
        QMAKE_CXXFLAGS += -IC:\ffmpeg-x64\include
        QMAKE_CFLAGS += -IC:\ffmpeg-x64\include
        LIBS += -LC:\ffmpeg-x64\lib
        QMAKE_CXXFLAGS += -IC:\fftw-3.3.5-x64
        QMAKE_CFLAGS += -IC:\fftw-3.3.5-x64
        LIBS += -LC:\fftw-3.3.5-x64
    } else {
        message("x86 build")
        QMAKE_CXXFLAGS += -IC:\ImageMagick\6.9.3-Q16-x86\include
        QMAKE_CFLAGS += -IC:\ImageMagick\6.9.3-Q16-x86\include
        LIBS += -LC:\ImageMagick\6.9.3-Q16-x86\lib
        QMAKE_CXXFLAGS += -IC:\ffmpeg-x86\include
        QMAKE_CFLAGS += -IC:\ffmpeg-x86\include
        LIBS += -LC:\ffmpeg-x86\lib
        QMAKE_CXXFLAGS += -IC:\fftw-3.3.5-x86
        QMAKE_CFLAGS += -IC:\fftw-3.3.5-x86
        LIBS += -LC:\fftw-3.3.5-x86
    }
    LIBS += -lCORE_RL_magick_ -lCORE_RL_wand_ -lCORE_RL_Magick++_
    LIBS += -lavformat -lavcodec -lavutil
    LIBS += -lfftw3-3
    RC_ICONS = icons/darkflow-256x256.ico \
        icons/darkflow-128x128.ico \
        icons/darkflow-96x96.ico \
        icons/darkflow-64x64.ico \
        icons/darkflow-48x48.ico \
        icons/darkflow-32x32.ico \
        icons/darkflow-24x24.ico
}

QMAKE_INCDIR += core operators algorithms scene ui setup

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
    operators/opcolor.cpp \
    ui/console.cpp \
    ui/preferences.cpp \
    algorithms/hdr.cpp \
    operators/ophdr.cpp \
    core/ports.cpp \
    core/posixspawn.cpp \
    ui/selectivelab.cpp \
    scene/processselectivelab.cpp \
    core/operatorparameterselectivelab.cpp \
    algorithms/selectivelabfilter.cpp \
    operators/opselectivelabfilter.cpp \
    ui/graphicsviewinteraction.cpp \
    core/ordinary.cpp \
    core/transformview.cpp \
    operators/opsave.cpp \
    scene/processdirectory.cpp \
    core/operatorparameterdirectory.cpp \
    operators/opairydisk.cpp \
    operators/opwienerdeconvolution.cpp \
    operators/workerwienerdeconvolution.cpp \
    algorithms/discretefouriertransform.cpp \
    operators/opdftforward.cpp \
    operators/opdftbackward.cpp \
    operators/opdwtforward.cpp \
    algorithms/atrouswavelettransform.cpp \
    operators/opdwtbackward.cpp \
    operators/opturnblack.cpp \
    operators/opdisk.cpp \
    operators/opphasecorrelationreg.cpp \
    operators/opwindowfunction.cpp \
    operators/opcolormap.cpp \
    operators/opstarfinder.cpp \
    operators/oppixelextrusionmapping.cpp

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
    operators/opcolor.h \
    ui/console.h \
    ui/preferences.h \
    algorithms/hdr.h \
    operators/ophdr.h \
    core/ports.h \
    core/posixspawn.h \
    core/darkflow.h \
    ui/selectivelab.h \
    scene/processselectivelab.h \
    core/operatorparameterselectivelab.h \
    algorithms/selectivelabfilter.h \
    operators/opselectivelabfilter.h \
    ui/graphicsviewinteraction.h \
    core/ordinary.h \
    core/transformview.h \
    operators/opsave.h \
    scene/processdirectory.h \
    core/operatorparameterdirectory.h \
    operators/opairydisk.h \
    operators/opwienerdeconvolution.h \
    operators/workerwienerdeconvolution.h \
    algorithms/discretefouriertransform.h \
    operators/opdftforward.h \
    operators/opdftbackward.h \
    operators/opdwtforward.h \
    algorithms/atrouswavelettransform.h \
    operators/opdwtbackward.h \
    operators/opturnblack.h \
    operators/opdisk.h \
    operators/opphasecorrelationreg.h \
    operators/opwindowfunction.h \
    operators/opcolormap.h \
    operators/opstarfinder.h \
    operators/oppixelextrusionmapping.h


FORMS    += \
    ui/aboutdialog.ui \
    ui/filesselection.ui \
    ui/mainwindow.ui \
    ui/projectproperties.ui \
    ui/visualization.ui \
    ui/slider.ui \
    ui/fullscreenview.ui \
    ui/console.ui \
    ui/preferences.ui \
    ui/selectivelab.ui

RESOURCES += \
    ui/resources.qrc

DISTFILES += \
    setup/darkflow-x64.iss \
    setup/darkflow-x86.iss \
    setup/darkflow-common.iss \
    setup/darkflow-version.iss \
    l10n/darkflow_fr.ts \
    setup/osx_prepare_bundle.sh

TRANSLATIONS = l10n/darkflow_fr.ts

unix:!macx {
    target.path = /usr/bin/
    INSTALLS += target
    df_icons.files = icons/darkflow.png
    df_icons.path = /usr/share/icons/hicolor/256x256/apps/
    INSTALLS += df_icons
    df_desktop_entry.files = setup/darkflow.desktop
    df_desktop_entry.path = /usr/share/applications/
    INSTALLS += df_desktop_entry
    df_mime_xml.files = setup/x-darkflow-project.xml
    df_mime_xml.path = /usr/share/mime/packages/
    INSTALLS += df_mime_xml
}
