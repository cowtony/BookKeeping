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
    void setTransaction(const Transaction& t);

private slots:
    void onCalendarWidgetSelectionChanged();
    void onDateTimeEditDateTimeChanged(const QDateTime& date_time);
    void on_lineEdit_Description_editingFinished();
    void onAccountCateChanged(QTableWidget* table_widget, int row);
    void on_pushButton_Split_clicked();
    void on_pushButton_Insert_clicked();

    void on_checkBox_RecursiveTransaction_stateChanged(int arg1);

signals:
    void insertTransactionFinished(AddTransaction *obj);

private:
    int insertTableRow(QTableWidget *tableWidget);
    void setTableRow(QTableWidget *tableWidget, Account account, const MoneyArray& amounts);

    Transaction getTransaction();

    Ui::AddTransaction* ui;
    Book& book_;
    int& user_id_;

    QMap<Account::Type, QTableWidget*> tableMap;
    int transaction_id_ = 0;
};

#endif // ADD_TRANSACTION_H
