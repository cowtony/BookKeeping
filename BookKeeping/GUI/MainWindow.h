#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateEdit>

#include "AccountManager.h"
#include "book_model.h"
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
  void refreshTable();

  void on_pushButtonDeleteTransactions_clicked();
  void accountCategoryChanged(const Account::TableType& table_type, const QString& category, QComboBox* name_combo_box);
  void on_tableView_transactions_doubleClicked(const QModelIndex &index);
  void setCategoryComboBox();

  void on_pushButton_MergeTransaction_clicked();

private:
  Ui::MainWindow *ui;

  AccountManager     *account_manager_;
  FinancialStatement *financial_statement_;

  Book book_;

  // Filter components:
  QDateEdit* start_date_;
  QDateEdit* end_date_;
  QVector<QComboBox*> category_combo_boxes_;
  QVector<QComboBox*> name_combo_boxes_;
  BookModel book_model_;
};

#endif // MAINWINDOW_H
