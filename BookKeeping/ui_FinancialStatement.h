/********************************************************************************
** Form generated from reading UI file 'FinancialStatement.ui'
**
** Created by: Qt User Interface Compiler version 5.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FINANCIALSTATEMENT_H
#define UI_FINANCIALSTATEMENT_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FinancialStatement
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QTreeWidget *treeWidget;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_Date;
    QDateTimeEdit *dateTimeEdit;
    QPushButton *pushButton_Query;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QRadioButton *radioButton_all;
    QRadioButton *radioButton_1;
    QRadioButton *radioButton_2;
    QPushButton *pushButtonExport;
    QPushButton *pushButtonShowMore;
    QPushButton *pushButton;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *FinancialStatement)
    {
        if (FinancialStatement->objectName().isEmpty())
            FinancialStatement->setObjectName(QString::fromUtf8("FinancialStatement"));
        FinancialStatement->resize(427, 717);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resource/Balance_Sheet.png"), QSize(), QIcon::Normal, QIcon::Off);
        FinancialStatement->setWindowIcon(icon);
        centralwidget = new QWidget(FinancialStatement);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        treeWidget = new QTreeWidget(centralwidget);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));

        gridLayout->addWidget(treeWidget, 4, 0, 1, 3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_Date = new QLabel(centralwidget);
        label_Date->setObjectName(QString::fromUtf8("label_Date"));

        horizontalLayout_2->addWidget(label_Date);

        dateTimeEdit = new QDateTimeEdit(centralwidget);
        dateTimeEdit->setObjectName(QString::fromUtf8("dateTimeEdit"));
        dateTimeEdit->setMaximumSize(QSize(16777215, 16777215));
        dateTimeEdit->setDateTime(QDateTime(QDate(2020, 1, 1), QTime(0, 0, 0)));

        horizontalLayout_2->addWidget(dateTimeEdit);

        pushButton_Query = new QPushButton(centralwidget);
        pushButton_Query->setObjectName(QString::fromUtf8("pushButton_Query"));

        horizontalLayout_2->addWidget(pushButton_Query);


        gridLayout->addLayout(horizontalLayout_2, 3, 0, 1, 3);

        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        radioButton_all = new QRadioButton(groupBox);
        radioButton_all->setObjectName(QString::fromUtf8("radioButton_all"));

        horizontalLayout->addWidget(radioButton_all);

        radioButton_1 = new QRadioButton(groupBox);
        radioButton_1->setObjectName(QString::fromUtf8("radioButton_1"));

        horizontalLayout->addWidget(radioButton_1);

        radioButton_2 = new QRadioButton(groupBox);
        radioButton_2->setObjectName(QString::fromUtf8("radioButton_2"));

        horizontalLayout->addWidget(radioButton_2);


        gridLayout->addWidget(groupBox, 0, 0, 3, 1);

        pushButtonExport = new QPushButton(centralwidget);
        pushButtonExport->setObjectName(QString::fromUtf8("pushButtonExport"));

        gridLayout->addWidget(pushButtonExport, 0, 1, 2, 2);

        pushButtonShowMore = new QPushButton(centralwidget);
        pushButtonShowMore->setObjectName(QString::fromUtf8("pushButtonShowMore"));

        gridLayout->addWidget(pushButtonShowMore, 2, 1, 1, 1);

        pushButton = new QPushButton(centralwidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        gridLayout->addWidget(pushButton, 2, 2, 1, 1);

        FinancialStatement->setCentralWidget(centralwidget);
        menubar = new QMenuBar(FinancialStatement);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 427, 21));
        FinancialStatement->setMenuBar(menubar);
        statusbar = new QStatusBar(FinancialStatement);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        FinancialStatement->setStatusBar(statusbar);

        retranslateUi(FinancialStatement);

        QMetaObject::connectSlotsByName(FinancialStatement);
    } // setupUi

    void retranslateUi(QMainWindow *FinancialStatement)
    {
        FinancialStatement->setWindowTitle(QApplication::translate("FinancialStatement", "Financial Statement", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeWidget->headerItem();
        ___qtreewidgetitem->setText(0, QApplication::translate("FinancialStatement", "Account", nullptr));
        label_Date->setText(QApplication::translate("FinancialStatement", " End  Date:", nullptr));
        dateTimeEdit->setDisplayFormat(QApplication::translate("FinancialStatement", "yyyy-MM-dd HH:mm:ss", nullptr));
        pushButton_Query->setText(QApplication::translate("FinancialStatement", "Query", nullptr));
        groupBox->setTitle(QApplication::translate("FinancialStatement", "Select View", nullptr));
        radioButton_all->setText(QApplication::translate("FinancialStatement", "All", nullptr));
        radioButton_1->setText(QApplication::translate("FinancialStatement", "Person 1", nullptr));
        radioButton_2->setText(QApplication::translate("FinancialStatement", "Person 2", nullptr));
        pushButtonExport->setText(QApplication::translate("FinancialStatement", "Copy to ClipBoard", nullptr));
        pushButtonShowMore->setText(QApplication::translate("FinancialStatement", "Show More", nullptr));
        pushButton->setText(QApplication::translate("FinancialStatement", "Show All", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FinancialStatement: public Ui_FinancialStatement {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FINANCIALSTATEMENT_H
