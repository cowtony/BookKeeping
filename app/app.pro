#-------------------------------------------------
#
# Project created by QtCreator 2018-02-20T10:00:07
#
#-------------------------------------------------

QT += core gui sql network charts

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

RC_ICONS = icons/App_Icon.ico

# INCLUDEPATH +=

HEADERS += \
    add_transaction/add_transaction.h \
    account_manager/account_manager.h \
    account_manager/account_tree_node.h \
    account_manager/accounts_model.h \
    add_transaction/no_scroll_combo_box.h \
    book/account.h \
    book/book.h \
    book/money.h \
    book/transaction.h \
    currency/currency.h \
    financial_statement/financial_statement.h \
    financial_statement/bar_chart.h \
    home_window/transactions_model.h \
    home_window/home_window.h \
    household_manager/household_manager.h \
    investment_analysis/investment_analysis.h \
    investment_analysis/investment_analyzer.h

SOURCES += main.cpp \
    add_transaction/add_transaction.cpp \
    account_manager/account_manager.cpp \
    account_manager/account_tree_node.cpp \
    account_manager/accounts_model.cpp \
    book/account.cpp \
    book/book.cpp \
    book/money.cpp \
    book/transaction.cpp \
    currency/currency.cpp \
    financial_statement/financial_statement.cpp \
    financial_statement/bar_chart.cpp \
    home_window/transactions_model.cpp \
    home_window/home_window.cpp \
    household_manager/household_manager.cpp \
    investment_analysis/investment_analysis.cpp \
    investment_analysis/investment_analyzer.cpp

FORMS += \
    add_transaction/add_transaction.ui \
    financial_statement/financial_statement.ui \
    household_manager/household_manager.ui \
    investment_analysis/investment_analysis.ui \
    home_window/home_window.ui

RESOURCES += \
    resources.qrc
