/********************************************************************************
** Form generated from reading UI file 'AddTransaction.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDTRANSACTION_H
#define UI_ADDTRANSACTION_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCalendarWidget>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AddTransaction
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QCalendarWidget *calendarWidget;
    QLabel *label_Currency;
    QComboBox *comboBox_Currency;
    QLineEdit *lineEdit_Description;
    QCheckBox *checkBox_RecursiveTransaction;
    QPushButton *pushButton_Insert;
    QDateTimeEdit *dateTimeEdit;
    QPushButton *pushButton_Split;
    QLabel *label_transactionDateTime;
    QDateEdit *dateEdit_nextTransaction;
    QLabel *label_Description;
    QSplitter *splitter;
    QTableWidget *tableWidget_Assets;
    QTableWidget *tableWidget_Liabilities;
    QTableWidget *tableWidget_Revenues;
    QTableWidget *tableWidget_Expenses;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *AddTransaction)
    {
        if (AddTransaction->objectName().isEmpty())
            AddTransaction->setObjectName(QString::fromUtf8("AddTransaction"));
        AddTransaction->resize(894, 546);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resource/AddTransaction.png"), QSize(), QIcon::Normal, QIcon::Off);
        AddTransaction->setWindowIcon(icon);
        centralwidget = new QWidget(AddTransaction);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout_2 = new QGridLayout(centralwidget);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setMaximumSize(QSize(300, 16777215));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        calendarWidget = new QCalendarWidget(groupBox);
        calendarWidget->setObjectName(QString::fromUtf8("calendarWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(calendarWidget->sizePolicy().hasHeightForWidth());
        calendarWidget->setSizePolicy(sizePolicy);
        calendarWidget->setMaximumSize(QSize(160000, 160000));

        gridLayout->addWidget(calendarWidget, 0, 0, 1, 2);

        label_Currency = new QLabel(groupBox);
        label_Currency->setObjectName(QString::fromUtf8("label_Currency"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_Currency->sizePolicy().hasHeightForWidth());
        label_Currency->setSizePolicy(sizePolicy1);
        QFont font;
        font.setPointSize(12);
        label_Currency->setFont(font);
        label_Currency->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_Currency, 7, 0, 1, 2);

        comboBox_Currency = new QComboBox(groupBox);
        comboBox_Currency->setObjectName(QString::fromUtf8("comboBox_Currency"));
        comboBox_Currency->setEnabled(false);

        gridLayout->addWidget(comboBox_Currency, 10, 0, 1, 2);

        lineEdit_Description = new QLineEdit(groupBox);
        lineEdit_Description->setObjectName(QString::fromUtf8("lineEdit_Description"));
        lineEdit_Description->setMaximumSize(QSize(16000, 20));

        gridLayout->addWidget(lineEdit_Description, 6, 0, 1, 2);

        checkBox_RecursiveTransaction = new QCheckBox(groupBox);
        checkBox_RecursiveTransaction->setObjectName(QString::fromUtf8("checkBox_RecursiveTransaction"));
        checkBox_RecursiveTransaction->setEnabled(true);

        gridLayout->addWidget(checkBox_RecursiveTransaction, 3, 0, 1, 2);

        pushButton_Insert = new QPushButton(groupBox);
        pushButton_Insert->setObjectName(QString::fromUtf8("pushButton_Insert"));

        gridLayout->addWidget(pushButton_Insert, 8, 1, 1, 1);

        dateTimeEdit = new QDateTimeEdit(groupBox);
        dateTimeEdit->setObjectName(QString::fromUtf8("dateTimeEdit"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(dateTimeEdit->sizePolicy().hasHeightForWidth());
        dateTimeEdit->setSizePolicy(sizePolicy2);
        dateTimeEdit->setMaximumSize(QSize(16000, 20));

        gridLayout->addWidget(dateTimeEdit, 2, 0, 1, 2);

        pushButton_Split = new QPushButton(groupBox);
        pushButton_Split->setObjectName(QString::fromUtf8("pushButton_Split"));

        gridLayout->addWidget(pushButton_Split, 8, 0, 1, 1);

        label_transactionDateTime = new QLabel(groupBox);
        label_transactionDateTime->setObjectName(QString::fromUtf8("label_transactionDateTime"));

        gridLayout->addWidget(label_transactionDateTime, 1, 0, 1, 2);

        dateEdit_nextTransaction = new QDateEdit(groupBox);
        dateEdit_nextTransaction->setObjectName(QString::fromUtf8("dateEdit_nextTransaction"));
        dateEdit_nextTransaction->setEnabled(true);

        gridLayout->addWidget(dateEdit_nextTransaction, 4, 0, 1, 2);

        label_Description = new QLabel(groupBox);
        label_Description->setObjectName(QString::fromUtf8("label_Description"));
        sizePolicy1.setHeightForWidth(label_Description->sizePolicy().hasHeightForWidth());
        label_Description->setSizePolicy(sizePolicy1);
        QFont font1;
        font1.setPointSize(8);
        label_Description->setFont(font1);
        label_Description->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout->addWidget(label_Description, 5, 0, 1, 2);


        gridLayout_2->addWidget(groupBox, 0, 0, 1, 1);

        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setMaximumSize(QSize(16777215, 16777215));
        splitter->setOrientation(Qt::Vertical);
        tableWidget_Assets = new QTableWidget(splitter);
        if (tableWidget_Assets->columnCount() < 3)
            tableWidget_Assets->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget_Assets->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget_Assets->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget_Assets->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        tableWidget_Assets->setObjectName(QString::fromUtf8("tableWidget_Assets"));
        tableWidget_Assets->setMinimumSize(QSize(0, 0));
        tableWidget_Assets->setMaximumSize(QSize(16777215, 16777215));
        splitter->addWidget(tableWidget_Assets);
        tableWidget_Liabilities = new QTableWidget(splitter);
        if (tableWidget_Liabilities->columnCount() < 3)
            tableWidget_Liabilities->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget_Liabilities->setHorizontalHeaderItem(0, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableWidget_Liabilities->setHorizontalHeaderItem(1, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tableWidget_Liabilities->setHorizontalHeaderItem(2, __qtablewidgetitem5);
        tableWidget_Liabilities->setObjectName(QString::fromUtf8("tableWidget_Liabilities"));
        tableWidget_Liabilities->setMinimumSize(QSize(0, 0));
        splitter->addWidget(tableWidget_Liabilities);
        tableWidget_Revenues = new QTableWidget(splitter);
        if (tableWidget_Revenues->columnCount() < 5)
            tableWidget_Revenues->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tableWidget_Revenues->setHorizontalHeaderItem(0, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tableWidget_Revenues->setHorizontalHeaderItem(1, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        tableWidget_Revenues->setHorizontalHeaderItem(2, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        tableWidget_Revenues->setHorizontalHeaderItem(3, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        tableWidget_Revenues->setHorizontalHeaderItem(4, __qtablewidgetitem10);
        tableWidget_Revenues->setObjectName(QString::fromUtf8("tableWidget_Revenues"));
        splitter->addWidget(tableWidget_Revenues);
        tableWidget_Expenses = new QTableWidget(splitter);
        if (tableWidget_Expenses->columnCount() < 5)
            tableWidget_Expenses->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        tableWidget_Expenses->setHorizontalHeaderItem(0, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        tableWidget_Expenses->setHorizontalHeaderItem(1, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        tableWidget_Expenses->setHorizontalHeaderItem(2, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        tableWidget_Expenses->setHorizontalHeaderItem(3, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        tableWidget_Expenses->setHorizontalHeaderItem(4, __qtablewidgetitem15);
        tableWidget_Expenses->setObjectName(QString::fromUtf8("tableWidget_Expenses"));
        splitter->addWidget(tableWidget_Expenses);

        gridLayout_2->addWidget(splitter, 0, 1, 1, 1);

        AddTransaction->setCentralWidget(centralwidget);
        menubar = new QMenuBar(AddTransaction);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 894, 21));
        AddTransaction->setMenuBar(menubar);
        statusbar = new QStatusBar(AddTransaction);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        AddTransaction->setStatusBar(statusbar);

        retranslateUi(AddTransaction);

        QMetaObject::connectSlotsByName(AddTransaction);
    } // setupUi

    void retranslateUi(QMainWindow *AddTransaction)
    {
        AddTransaction->setWindowTitle(QApplication::translate("AddTransaction", "Add Transaction", nullptr));
        groupBox->setTitle(QApplication::translate("AddTransaction", "Transaction Information", nullptr));
        label_Currency->setText(QApplication::translate("AddTransaction", "Currency:", nullptr));
        checkBox_RecursiveTransaction->setText(QApplication::translate("AddTransaction", "Next Transaction", nullptr));
        pushButton_Insert->setText(QApplication::translate("AddTransaction", "INSERT", nullptr));
        dateTimeEdit->setDisplayFormat(QApplication::translate("AddTransaction", "yyyy-MM-dd HH:mm", nullptr));
        pushButton_Split->setText(QApplication::translate("AddTransaction", "Split Fill", nullptr));
        label_transactionDateTime->setText(QApplication::translate("AddTransaction", "Transaction Date Time:", nullptr));
        dateEdit_nextTransaction->setDisplayFormat(QApplication::translate("AddTransaction", "yyyy-MM-dd", nullptr));
        label_Description->setText(QApplication::translate("AddTransaction", "Description:", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget_Assets->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("AddTransaction", "Category", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget_Assets->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("AddTransaction", "Asset Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget_Assets->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("AddTransaction", "Amount", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget_Liabilities->horizontalHeaderItem(0);
        ___qtablewidgetitem3->setText(QApplication::translate("AddTransaction", "Category", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableWidget_Liabilities->horizontalHeaderItem(1);
        ___qtablewidgetitem4->setText(QApplication::translate("AddTransaction", "Liability Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tableWidget_Liabilities->horizontalHeaderItem(2);
        ___qtablewidgetitem5->setText(QApplication::translate("AddTransaction", "Amount", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tableWidget_Revenues->horizontalHeaderItem(0);
        ___qtablewidgetitem6->setText(QApplication::translate("AddTransaction", "Category", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tableWidget_Revenues->horizontalHeaderItem(1);
        ___qtablewidgetitem7->setText(QApplication::translate("AddTransaction", "Revenue Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = tableWidget_Revenues->horizontalHeaderItem(2);
        ___qtablewidgetitem8->setText(QApplication::translate("AddTransaction", "Person 1", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = tableWidget_Revenues->horizontalHeaderItem(3);
        ___qtablewidgetitem9->setText(QApplication::translate("AddTransaction", "Person 2", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = tableWidget_Revenues->horizontalHeaderItem(4);
        ___qtablewidgetitem10->setText(QApplication::translate("AddTransaction", "Person 3", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = tableWidget_Expenses->horizontalHeaderItem(0);
        ___qtablewidgetitem11->setText(QApplication::translate("AddTransaction", "Category", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = tableWidget_Expenses->horizontalHeaderItem(1);
        ___qtablewidgetitem12->setText(QApplication::translate("AddTransaction", "Expense Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem13 = tableWidget_Expenses->horizontalHeaderItem(2);
        ___qtablewidgetitem13->setText(QApplication::translate("AddTransaction", "Person 1", nullptr));
        QTableWidgetItem *___qtablewidgetitem14 = tableWidget_Expenses->horizontalHeaderItem(3);
        ___qtablewidgetitem14->setText(QApplication::translate("AddTransaction", "Person 2", nullptr));
        QTableWidgetItem *___qtablewidgetitem15 = tableWidget_Expenses->horizontalHeaderItem(4);
        ___qtablewidgetitem15->setText(QApplication::translate("AddTransaction", "Person 3", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AddTransaction: public Ui_AddTransaction {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDTRANSACTION_H
