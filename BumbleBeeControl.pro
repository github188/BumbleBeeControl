#-------------------------------------------------
#
# Project created by QtCreator 2016-10-20T15:41:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BumbleBeeControl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        settings.cpp \
        cqtopencvviewergl.cpp \
        configs.cpp \
        genlcamcap.cpp \
        imagebuffer.cpp \
        opencvsink.cpp \
        hdf5sink.cpp

HEADERS  += mainwindow.h \
        settings.h \
        ControlCAN.h \
        cqtopencvviewergl.h \
        configs.h \
        genlcamcap.h \
        imagebuffer.h \
        imagepacket.h \
        opencvsink.h \
        hdf5sink.h \
    stimulusparams.h

FORMS    += \
        mainwindow.ui \
        settings.ui \
        genlcamcap.ui

RESOURCES += \
        state_pic.qrc


#GenlCam lib path
win32: LIBS += -L$$PWD/videoinput/genlcam/Lib/x64/ -lMVSDKmd

INCLUDEPATH += $$PWD/videoinput/genlcam/Include
DEPENDPATH += $$PWD/videoinput/genlcam/Include

#hdf5 lib path
win32: LIBS += -L$$PWD/../../develop/hdf5/1.10.0-patch1/lib/ -lhdf5 \
                                                               -lhdf5_cpp \
                                                               -lszip \
                                                               -lzlib
INCLUDEPATH += $$PWD/../../develop/hdf5/1.10.0-patch1/include
DEPENDPATH += $$PWD/../../develop/hdf5/1.10.0-patch1/include

#boost lib path
INCLUDEPATH += $$PWD/../../develop/boost/boost_1_62_0

#UsbCan lib path
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/./ -lControlCAN
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/./ -lControlCAN

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.


#Opencv lib path
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../develop/opencv/3.2/build/x64/vc14/lib/ -lopencv_world320
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../develop/opencv/3.2/build/x64/vc14/lib/ -lopencv_world320d

INCLUDEPATH += $$PWD/../../develop/opencv/3.2/build/include
DEPENDPATH += $$PWD/../../develop/opencv/3.2/build/include
