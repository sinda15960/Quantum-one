QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Atelier_Connexion
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           residents.cpp \
           connection.cpp

HEADERS += mainwindow.h \
           residents.h \
           connection.h

FORMS += mainwindow.ui

# Si tu utilises SQL
QT += sql
