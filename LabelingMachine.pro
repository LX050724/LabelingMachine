QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Src/ClientUI.cpp \
    Src/aboutwindow.cpp \
    Src/bandbox.cpp \
    Src/bandboxview.cpp \
    Src/classeditor.cpp \
    Src/helpwindow.cpp \
    Src/imagedata.cpp \
    Src/TCP_Server.cpp \
    Src/labelingmaching.cpp \
    Src/main.cpp \
    Src/mainwindow.cpp \
    Src/module.cpp \
    Src/openvideo.cpp \
    Src/TCP_Client.cpp \
    Src/ServerUI.cpp

HEADERS += \
    Inc/ClientUI.h \
    Inc/aboutwindow.h \
    Inc/bandbox.h \
    Inc/bandboxview.h \
    Inc/classeditor.h \
    Inc/helpwindow.h \
    Inc/imagedata.h \
    Inc/TCP_Server.h \
    Inc/Labels.h \
    Inc/labelingmaching.h \
    Inc/mainwindow.h \
    Inc/module.h \
    Inc/openvideo.h \
    Inc/TCP_Client.h \ \
    Inc/ServerUI.h

FORMS += \
    Src/ClientUI.ui \
    Src/aboutwindow.ui \
    Src/classeditor.ui \
    Src/helpwindow.ui \
    Src/mainwindow.ui \
    Src/module.ui \
    Src/openvideo.ui \
    Src/ServerUI.ui

TRANSLATIONS += \
    LabelingMachine_zh_CN.ts

RC_ICONS = bitbug_favicon.ico

RESOURCES += \
    Resource.qrc

INCLUDEPATH += /usr/local/include/opencv4/opencv /usr/local/include/opencv4 ./Inc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv4
