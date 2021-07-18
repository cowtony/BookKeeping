#ifndef ADDTRANSACTION_H
#define ADDTRANSACTION_H

#include <QMainWindow>
#include <QComboBox>
#include <QTableWidgetItem>
#include <QPushButton>
#include "Book.h"

namespace Ui {
class AddTransaction;
}

class AddTransaction : public QMainWindow {
  Q_OBJECT

public:
  explicit AddTransaction(Book& book, QWidget *parent);
  ~AddTransaction();

  void initialization();
  void setTransaction(const Transaction &t);

private slots:
  void on_calendarWidget_selectionChanged();
  void on_dateTimeEdit_dateTimeChanged(const QDateTime &dateTime_);
  void on_lineEdit_Description_editingFinished();

  void onAccountNameChanged(QTableWidget* tableWidget, int row);
  void onAccountCateChanged(QTableWidget* tableWidget, int row);

  void on_pushButton_Split_clicked();
  void on_pushButton_Insert_clicked();

  void on_checkBox_RecursiveTransaction_stateChanged(int arg1);

signals:
  void insertTransactionFinished(AddTransaction *obj);

private:
  Ui::AddTransaction *ui;
  Book& book_;

  QMap<Account::TableType, QTableWidget*> tableMap;

  QDateTime replacedDateTime;

  int insertTableRow(QTableWidget *tableWidget);
  void setTableRow(QTableWidget *tableWidget, Account account, const MoneyArray& amounts);

  Transaction getTransaction();
};

#endif // ADDTRANSACTION_H
