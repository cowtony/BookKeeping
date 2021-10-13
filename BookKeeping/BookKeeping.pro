#-------------------------------------------------
#
# Project created by QtCreator 2018-02-20T10:00:07
#
#-------------------------------------------------

QT += core gui
QT += sql
QT += charts
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BookKeeping
TEMPLATE = app
CONFIG += static

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RC_ICONS = Resource/App_Icon.ico

INCLUDEPATH += GUI \
    main_window \
    account_manager \
    Book

SOURCES += \
    GUI/BarChart.cpp \
    GUI/FinancialStatement.cpp \
    GUI/InvestmentAnalysis.cpp \
    account_manager/account_manager.cpp \
    account_manager/account_tree_node.cpp \
    account_manager/accounts_model.cpp \
    main.cpp \
    GUI/AddTransaction.cpp \
    main_window/book_model.cpp \
    main_window/main_window.cpp

FORMS += \
    GUI/AddTransaction.ui \
    GUI/FinancialStatement.ui \
    GUI/InvestmentAnalysis.ui \
    account_manager/account_manager.ui \
    main_window/main_window.ui

HEADERS += \
    GUI/AddTransaction.h \
    GUI/BarChart.h \
    GUI/FinancialStatement.h \
    GUI/InvestmentAnalysis.h \
    account_manager/account_manager.h \
    account_manager/account_tree_node.h \
    account_manager/accounts_model.h \
    main_window/book_model.h \
    main_window/main_window.h

RESOURCES += \
    resources.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../BOOK/release/ -lBook
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../BOOK/debug/ -lBook
else:unix: LIBS += -L$$OUT_PWD/../Book/ -lBook
INCLUDEPATH += $$PWD/../Book
DEPENDPATH += $$PWD/../Book

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Currency/release/ -lCurrency
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Currency/debug/ -lCurrency
else:unix: LIBS += -L$$OUT_PWD/../Currency/ -lCurrency
INCLUDEPATH += $$PWD/../Currency
DEPENDPATH += $$PWD/../Currency
