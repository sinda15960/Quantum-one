QT       += core gui network serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql
QT += core gui sql printsupport charts network

# C++ standard - use C++11 for Qt 5.9.9 compatibility
lessThan(QT_MAJOR_VERSION, 6): QMAKE_CXXFLAGS += -std=c++11
greaterThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++17

# QrCode.cpp removed - using SimpleQRCode instead (no encoding issues)

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
    crud.cpp \
    vehicle.cpp \
    statisticsdialog.cpp \
    simpleqrcode.cpp \
    service.cpp \
    arduino.cpp

HEADERS += \
    connexion.h \
    employe.h \
    gresident.h \
    resident.h \
    crud.h \
    vehicle.h \
    statisticsdialog.h \
    simpleqrcode.h \
    service.h \
    arduino.h

FORMS += \
    gresident.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
