/********************************************************************************
** Form generated from reading UI file 'AccountManager.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ACCOUNTMANAGER_H
#define UI_ACCOUNTMANAGER_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AccountManager
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QPushButton *pushButton_Add;
    QPushButton *pushButton_Rename;
    QPushButton *pushButton_Delete;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *AccountManager)
    {
        if (AccountManager->objectName().isEmpty())
            AccountManager->setObjectName(QString::fromUtf8("AccountManager"));
        AccountManager->resize(344, 371);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resource/Transactions.png"), QSize(), QIcon::Normal, QIcon::Off);
        AccountManager->setWindowIcon(icon);
        centralwidget = new QWidget(AccountManager);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        pushButton_Add = new QPushButton(centralwidget);
        pushButton_Add->setObjectName(QString::fromUtf8("pushButton_Add"));
        pushButton_Add->setEnabled(true);
        pushButton_Add->setCheckable(false);

        gridLayout->addWidget(pushButton_Add, 1, 0, 1, 1);

        pushButton_Rename = new QPushButton(centralwidget);
        pushButton_Rename->setObjectName(QString::fromUtf8("pushButton_Rename"));

        gridLayout->addWidget(pushButton_Rename, 1, 1, 1, 1);

        pushButton_Delete = new QPushButton(centralwidget);
        pushButton_Delete->setObjectName(QString::fromUtf8("pushButton_Delete"));
        pushButton_Delete->setEnabled(true);

        gridLayout->addWidget(pushButton_Delete, 1, 2, 1, 1);

        AccountManager->setCentralWidget(centralwidget);
        menubar = new QMenuBar(AccountManager);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 344, 21));
        AccountManager->setMenuBar(menubar);
        statusbar = new QStatusBar(AccountManager);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        AccountManager->setStatusBar(statusbar);

        retranslateUi(AccountManager);

        QMetaObject::connectSlotsByName(AccountManager);
    } // setupUi

    void retranslateUi(QMainWindow *AccountManager)
    {
        AccountManager->setWindowTitle(QApplication::translate("AccountManager", "Account Manager", nullptr));
        pushButton_Add->setText(QApplication::translate("AccountManager", "Add", nullptr));
        pushButton_Rename->setText(QApplication::translate("AccountManager", "Rename", nullptr));
        pushButton_Delete->setText(QApplication::translate("AccountManager", "Delete", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AccountManager: public Ui_AccountManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ACCOUNTMANAGER_H
