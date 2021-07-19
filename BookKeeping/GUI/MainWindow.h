#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateEdit>
#include "AccountManager.h"
#include "FinancialStatement.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  void on_actionAddTransaction_triggered();
  void on_actionAccountManager_triggered();
  void on_actionFinancialStatement_triggered();
  void on_actionInvestmentAnalysis_triggered();
  void on_actionTransactionValidation_triggered();

  // Show all filtered transactions.
  void displayTransactions();

  void on_pushButtonDeleteTransactions_clicked();
  void accountCategoryChanged(const Account::TableType& table_type, const QString& category, QComboBox* name_combo_box);
  void on_tableWidget_transactions_cellDoubleClicked(int row, int column);
  void setCategoryComboBox();

  void on_pushButton_MergeTransaction_clicked();

private:
  QMap<QDateTime, QString> getSelectedTransactionInfos() const;

  Ui::MainWindow *ui;

  AccountManager     *account_manager_;
  FinancialStatement *financial_statement_;

  Book book_;

  const QVector<Account::TableType> kTableList = {Account::Expense, Account::Revenue, Account::Asset, Account::Liability};

  // Filter components:
  QDateEdit* start_date_;
  QDateEdit* end_date_;
  QVector<QComboBox*> category_combo_boxes_;
  QVector<QComboBox*> name_combo_boxes_;
};

#endif // MAINWINDOW_H
