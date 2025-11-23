QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql
QT += core gui sql printsupport multimedia websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    connexion.cpp \
    employe.cpp \
    main.cpp \
    gresident.cpp \
    resident.cpp \
    serviceitem.cpp \
    crud.cpp \
    vehicle.cpp \
    smartavailability.cpp \
    voicetotext.cpp

HEADERS += \
    connexion.h \
    employe.h \
    gresident.h \
    resident.h \
    serviceitem.h \
    crud.h \
    vehicle.h \
    smartavailability.h \
    voicetotext.h

FORMS += \
    gresident.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
