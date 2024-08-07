#ifndef ADD_TRANSACTION_H
#define ADD_TRANSACTION_H

#include <QMainWindow>
#include <QComboBox>
#include <QTableWidgetItem>
#include <QPushButton>

#include "book/book.h"

namespace Ui {
class AddTransaction;
}

class AddTransaction : public QMainWindow {
    Q_OBJECT

public:
    explicit AddTransaction(QWidget *parent);
    ~AddTransaction();

    void initialization();
    void setTransaction(const Transaction& transaction);

    static void setupTimeZoneComboBox(QComboBox*);

private slots:
    void onCalendarWidgetSelectionChanged();
    void onDateTimeEditDateTimeChanged(const QDateTime& date_time);
    void on_comboBox_TimeZone_currentTextChanged(const QString &arg1);
    void on_lineEdit_Description_editingFinished();
    void onAccountCateChanged(QTableWidget* table_widget, int row);
    void on_pushButton_Split_clicked();
    void on_pushButton_Insert_clicked();

    void on_checkBox_RecursiveTransaction_stateChanged(int arg1);

signals:
    void insertTransactionFinished(QDate earliest_modified_utc_date);

private:
    int insertTableRow(QTableWidget *tableWidget);
    void setTableRow(QTableWidget* tableWidget, const Account& account, const HouseholdMoney& household_money);
    Transaction getTransaction();

    Ui::AddTransaction* ui;
    Book& book_;
    int& user_id_;

    QMap<Account::Type, QTableWidget*> table_widgets_;
    QMap<QString, int> household_to_column_;
    int transaction_id_ = -1;
};

#endif // ADD_TRANSACTION_H
