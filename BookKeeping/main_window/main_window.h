#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QDateEdit>

#include "account_manager.h"
#include "book_model.h"
#include "financial_statement.h"
#include "currency.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

  public slots:
    void refreshTable();  // Show all filtered transactions.
    void setCategoryComboBox();

  protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void closeEvent(QCloseEvent *event) override;

  private slots:
    void on_actionAddTransaction_triggered();
    void on_actionAccountManager_triggered();
    void on_actionFinancialStatement_triggered();
    void on_actionInvestmentAnalysis_triggered();
    void on_actionTransactionValidation_triggered();

    void on_pushButtonDeleteTransactions_clicked();
    void accountCategoryChanged(const Account::Type& table_type, const QString& category, QComboBox* name_combo_box);
    void on_tableView_transactions_doubleClicked(const QModelIndex &index);

    void on_pushButton_MergeTransaction_clicked();

  private:
    void resizeColumns();

    QSharedPointer<Ui::MainWindow> ui_;

    QSharedPointer<AccountManager>     account_manager_;
    QSharedPointer<FinancialStatement> financial_statement_;

    Book book_;
    BookModel book_model_;

    // Filter components:
    QDateEdit* start_date_;
    QDateEdit* end_date_;
    QLineEdit* description_;
    QVector<QComboBox*> category_combo_boxes_;
    QVector<QComboBox*> name_combo_boxes_;

    // To be able to reference `book_`.
    friend class AccountManager;
    friend class AddTransaction;
    friend class FinancialStatement;
};

#endif // MAIN_WINDOW_H
