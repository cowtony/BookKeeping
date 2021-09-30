/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
//#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionAddTransaction;
    QAction *actionAccountManager;
    QAction *actionInvestmentAnalysis;
    QAction *actionFinancialStatement;
    QAction *actionTransactionValidation;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QTableWidget *tableWidget_transactions;
    QPushButton *pushButtonDeleteTransactions;
    QPushButton *pushButton_MergeTransaction;
    QMenuBar *menuBar;
    QMenu *menuFunctions;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1475, 709);
        actionAddTransaction = new QAction(MainWindow);
        actionAddTransaction->setObjectName(QString::fromUtf8("actionAddTransaction"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resource/AddTransaction.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAddTransaction->setIcon(icon);
        actionAccountManager = new QAction(MainWindow);
        actionAccountManager->setObjectName(QString::fromUtf8("actionAccountManager"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/resource/Transactions.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAccountManager->setIcon(icon1);
        actionInvestmentAnalysis = new QAction(MainWindow);
        actionInvestmentAnalysis->setObjectName(QString::fromUtf8("actionInvestmentAnalysis"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/resource/Investment_Analysis.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionInvestmentAnalysis->setIcon(icon2);
        actionFinancialStatement = new QAction(MainWindow);
        actionFinancialStatement->setObjectName(QString::fromUtf8("actionFinancialStatement"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/resource/Balance_Sheet.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionFinancialStatement->setIcon(icon3);
        actionTransactionValidation = new QAction(MainWindow);
        actionTransactionValidation->setObjectName(QString::fromUtf8("actionTransactionValidation"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        tableWidget_transactions = new QTableWidget(centralWidget);
        if (tableWidget_transactions->columnCount() < 6)
            tableWidget_transactions->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget_transactions->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget_transactions->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget_transactions->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget_transactions->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidget_transactions->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidget_transactions->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        if (tableWidget_transactions->rowCount() < 3)
            tableWidget_transactions->setRowCount(3);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableWidget_transactions->setVerticalHeaderItem(0, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableWidget_transactions->setVerticalHeaderItem(1, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableWidget_transactions->setVerticalHeaderItem(2, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableWidget_transactions->setItem(0, 0, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableWidget_transactions->setItem(0, 1, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        tableWidget_transactions->setItem(0, 2, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tableWidget_transactions->setItem(0, 3, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        tableWidget_transactions->setItem(0, 4, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        tableWidget_transactions->setItem(0, 5, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        tableWidget_transactions->setItem(1, 0, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        tableWidget_transactions->setItem(1, 1, __qtablewidgetitem16);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        tableWidget_transactions->setItem(1, 2, __qtablewidgetitem17);
        QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
        tableWidget_transactions->setItem(1, 3, __qtablewidgetitem18);
        QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
        tableWidget_transactions->setItem(1, 4, __qtablewidgetitem19);
        QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
        tableWidget_transactions->setItem(1, 5, __qtablewidgetitem20);
        QTableWidgetItem *__qtablewidgetitem21 = new QTableWidgetItem();
        tableWidget_transactions->setItem(2, 0, __qtablewidgetitem21);
        tableWidget_transactions->setObjectName(QString::fromUtf8("tableWidget_transactions"));

        gridLayout->addWidget(tableWidget_transactions, 0, 0, 1, 2);

        pushButtonDeleteTransactions = new QPushButton(centralWidget);
        pushButtonDeleteTransactions->setObjectName(QString::fromUtf8("pushButtonDeleteTransactions"));

        gridLayout->addWidget(pushButtonDeleteTransactions, 1, 1, 1, 1);

        pushButton_MergeTransaction = new QPushButton(centralWidget);
        pushButton_MergeTransaction->setObjectName(QString::fromUtf8("pushButton_MergeTransaction"));

        gridLayout->addWidget(pushButton_MergeTransaction, 1, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1475, 21));
        menuFunctions = new QMenu(menuBar);
        menuFunctions->setObjectName(QString::fromUtf8("menuFunctions"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFunctions->menuAction());
        menuFunctions->addAction(actionAddTransaction);
        menuFunctions->addAction(actionAccountManager);
        menuFunctions->addAction(actionFinancialStatement);
        menuFunctions->addAction(actionInvestmentAnalysis);
        menuFunctions->addAction(actionTransactionValidation);
        mainToolBar->addAction(actionAddTransaction);
        mainToolBar->addAction(actionAccountManager);
        mainToolBar->addAction(actionFinancialStatement);
        mainToolBar->addAction(actionInvestmentAnalysis);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Book Keeping", nullptr));
        actionAddTransaction->setText(QApplication::translate("MainWindow", "Add Transaction", nullptr));
        actionAccountManager->setText(QApplication::translate("MainWindow", "Account Editor", nullptr));
        actionInvestmentAnalysis->setText(QApplication::translate("MainWindow", "Investment Analysis", nullptr));
#ifndef QT_NO_TOOLTIP
        actionInvestmentAnalysis->setToolTip(QApplication::translate("MainWindow", "Investment Analysis", nullptr));
#endif // QT_NO_TOOLTIP
        actionFinancialStatement->setText(QApplication::translate("MainWindow", "Financial Statement", nullptr));
#ifndef QT_NO_TOOLTIP
        actionFinancialStatement->setToolTip(QApplication::translate("MainWindow", "Financial Statement", nullptr));
#endif // QT_NO_TOOLTIP
        actionTransactionValidation->setText(QApplication::translate("MainWindow", "Transaction Validation", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget_transactions->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("MainWindow", "Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget_transactions->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("MainWindow", "Description", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget_transactions->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("MainWindow", "Expense", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget_transactions->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QApplication::translate("MainWindow", "Revenue", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidget_transactions->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QApplication::translate("MainWindow", "Asset", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidget_transactions->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QApplication::translate("MainWindow", "Liability", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidget_transactions->verticalHeaderItem(0);
        ___qtablewidgetitem6->setText(QApplication::translate("MainWindow", "From:", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidget_transactions->verticalHeaderItem(1);
        ___qtablewidgetitem7->setText(QApplication::translate("MainWindow", "To:", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidget_transactions->verticalHeaderItem(2);
        ___qtablewidgetitem8->setText(QApplication::translate("MainWindow", "1", nullptr));

        const bool __sortingEnabled = tableWidget_transactions->isSortingEnabled();
        tableWidget_transactions->setSortingEnabled(false);
        QTableWidgetItem *___qtablewidgetitem9 = tableWidget_transactions->item(0, 0);
        ___qtablewidgetitem9->setText(QApplication::translate("MainWindow", "Start Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = tableWidget_transactions->item(0, 1);
        ___qtablewidgetitem10->setText(QApplication::translate("MainWindow", "a", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = tableWidget_transactions->item(0, 2);
        ___qtablewidgetitem11->setText(QApplication::translate("MainWindow", "ExpenseCategory", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = tableWidget_transactions->item(0, 3);
        ___qtablewidgetitem12->setText(QApplication::translate("MainWindow", "RevenueCategory", nullptr));
        QTableWidgetItem *___qtablewidgetitem13 = tableWidget_transactions->item(0, 4);
        ___qtablewidgetitem13->setText(QApplication::translate("MainWindow", "AccountCategory", nullptr));
        QTableWidgetItem *___qtablewidgetitem14 = tableWidget_transactions->item(0, 5);
        ___qtablewidgetitem14->setText(QApplication::translate("MainWindow", "LiabilityCategory", nullptr));
        QTableWidgetItem *___qtablewidgetitem15 = tableWidget_transactions->item(1, 0);
        ___qtablewidgetitem15->setText(QApplication::translate("MainWindow", "End Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem16 = tableWidget_transactions->item(1, 1);
        ___qtablewidgetitem16->setText(QApplication::translate("MainWindow", "b", nullptr));
        QTableWidgetItem *___qtablewidgetitem17 = tableWidget_transactions->item(1, 2);
        ___qtablewidgetitem17->setText(QApplication::translate("MainWindow", "ExpenseName", nullptr));
        QTableWidgetItem *___qtablewidgetitem18 = tableWidget_transactions->item(1, 3);
        ___qtablewidgetitem18->setText(QApplication::translate("MainWindow", "RevenueName", nullptr));
        QTableWidgetItem *___qtablewidgetitem19 = tableWidget_transactions->item(1, 4);
        ___qtablewidgetitem19->setText(QApplication::translate("MainWindow", "AccountName", nullptr));
        QTableWidgetItem *___qtablewidgetitem20 = tableWidget_transactions->item(1, 5);
        ___qtablewidgetitem20->setText(QApplication::translate("MainWindow", "LiabilityName", nullptr));
        tableWidget_transactions->setSortingEnabled(__sortingEnabled);

        pushButtonDeleteTransactions->setText(QApplication::translate("MainWindow", "Delete Transactions", nullptr));
        pushButton_MergeTransaction->setText(QApplication::translate("MainWindow", "Merge Transaction", nullptr));
        menuFunctions->setTitle(QApplication::translate("MainWindow", "Functions", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
