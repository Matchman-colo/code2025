QT       += core gui
QT       += mqtt  network
QT       += mqtt
INCLUDEPATH += $$PWD
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

RC_ICONS=$$PWD/resource/1451.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cleanlogwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    mymqttclient.cpp \
    settingwindow.cpp

HEADERS += \
    cleanlogwindow.h \
    mainwindow.h \
    mymqttclient.h \
    publishheader.h \
    settingwindow.h

FORMS += \
    cleanlogwindow.ui \
    mainwindow.ui \
    settingwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
QT += core gui mqtt

DISTFILES += \
    cleanlog_1.txt \
    cleanlog_2.txt
