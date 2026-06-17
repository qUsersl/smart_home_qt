QT       += core gui mqtt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    widget.cpp \
    ledpage.cpp \
    fanpage.cpp \
    doorlockpage.cpp \
    temhumpage.cpp \
    soilpage.cpp \
    peoplepage.cpp \
    co2page.cpp \
    pm25page.cpp \
    sunshadepage.cpp \
    flamgaspage.cpp \
    firepage.cpp \
    alarmpage.cpp \
    autocontrol.cpp \
    mqttconfig.cpp

HEADERS += \
    widget.h \
    ledpage.h \
    fanpage.h \
    doorlockpage.h \
    temhumpage.h \
    soilpage.h \
    peoplepage.h \
    co2page.h \
    pm25page.h \
    sunshadepage.h \
    flamgaspage.h \
    firepage.h \
    alarmpage.h \
    autocontrol.h \
    mqttconfig.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
