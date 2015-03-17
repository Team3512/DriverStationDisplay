#-------------------------------------------------
#
# Project created by QtCreator 2015-01-01T21:16:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = DriverStationDisplay
TEMPLATE = app
CONFIG += c++14

LIBS += -ljpeg

SOURCES +=\
    src/Main.cpp \
    src/MainWindow.cpp \
    src/Settings.cpp \
    src/Util.cpp \
    src/MJPEG/MjpegClient.cpp \
    src/MJPEG/mjpeg_sck.cpp \
    src/MJPEG/mjpeg_sck_selector.cpp \
    src/MJPEG/MjpegStream.cpp \
    src/MJPEG/win32_socketpair.c \
    src/NetUpdate/Drawable.cpp \
    src/NetUpdate/NetUpdate.cpp \
    src/NetUpdate/ProgressBar.cpp \
    src/NetUpdate/RectangleShape.cpp \
    src/NetUpdate/StatusLight.cpp \
    src/NetUpdate/Text.cpp \
    src/NetUpdate/UIFont.cpp \
    src/NetUpdate/NetValue.cpp

HEADERS  += \
    src/MainWindow.hpp \
    src/Settings.hpp \
    src/Util.hpp \
    src/Util.inl \
    src/MJPEG/MjpegClient.hpp \
    src/MJPEG/mjpeg_sck.hpp \
    src/MJPEG/mjpeg_sck_selector.hpp \
    src/MJPEG/MjpegStream.hpp \
    src/MJPEG/win32_socketpair.h \
    src/MJPEG/WindowCallbacks.hpp \
    src/NetUpdate/Drawable.hpp \
    src/NetUpdate/NetUpdate.hpp \
    src/NetUpdate/ProgressBar.hpp \
    src/NetUpdate/RectangleShape.hpp \
    src/NetUpdate/StatusLight.hpp \
    src/NetUpdate/Text.hpp \
    src/NetUpdate/UIFont.hpp \
    src/NetUpdate/NetValue.hpp

RESOURCES += \
    DriverStationDisplay.qrc

DISTFILES += \
    Resources.rc
