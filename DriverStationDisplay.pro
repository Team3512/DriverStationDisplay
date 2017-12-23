QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = DriverStationDisplay
TEMPLATE = app
CONFIG += c++14

win32:LIBS += -lws2_32

CONFIG += debug_and_release

SOURCES += \
    src/Main.cpp \
    src/MainWindow.cpp \
    src/Settings.cpp \
    src/Util.cpp \
    src/MJPEG/ClientBase.cpp \
    src/MJPEG/MjpegClient.cpp \
    src/MJPEG/mjpeg_sck.cpp \
    src/MJPEG/mjpeg_sck_selector.cpp \
    src/MJPEG/VideoStream.cpp \
    src/MJPEG/win32_socketpair.c \
    src/NetWidgets/NetWidget.cpp \
    src/NetWidgets/CircleWidget.cpp \
    src/NetWidgets/ProgressBar.cpp \
    src/NetWidgets/StatusLight.cpp \
    src/NetWidgets/Text.cpp \
    src/NetWidgets/NetEntry.cpp

HEADERS  += \
    src/MainWindow.hpp \
    src/Settings.hpp \
    src/Util.hpp \
    src/Util.inl \
    src/MJPEG/ClientBase.hpp \
    src/MJPEG/MjpegClient.hpp \
    src/MJPEG/mjpeg_sck.hpp \
    src/MJPEG/mjpeg_sck_selector.hpp \
    src/MJPEG/VideoStream.hpp \
    src/MJPEG/win32_socketpair.h \
    src/MJPEG/WindowCallbacks.hpp \
    src/NetWidgets/NetWidget.hpp \
    src/NetWidgets/CircleWidget.hpp \
    src/NetWidgets/ProgressBar.hpp \
    src/NetWidgets/StatusLight.hpp \
    src/NetWidgets/Text.hpp \
    src/NetWidgets/NetEntry.hpp

RESOURCES += \
    DriverStationDisplay.qrc

DISTFILES += \
    Resources.rc

LIBS += -ljpeg
