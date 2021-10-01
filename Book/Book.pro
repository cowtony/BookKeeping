#-------------------------------------------------
#
# Project created by QtCreator 2019-04-17T09:41:59
#
#-------------------------------------------------

QT -= gui
QT += sql network

TARGET = Book
TEMPLATE = lib

DEFINES += BOOK_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Account.cpp \
    Transaction.cpp \
    Book.cpp \
    Money.cpp

HEADERS += \
    Account.h \
    Transaction.h \
    Book.h \
    Money.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += Book.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Currency/release/ -lCurrency
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Currency/debug/ -lCurrency
else:unix: LIBS += -L$$OUT_PWD/../Currency/ -lCurrency

INCLUDEPATH += $$PWD/../Currency
DEPENDPATH += $$PWD/../Currency
