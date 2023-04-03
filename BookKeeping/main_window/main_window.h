#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QDateEdit>

#include "account_manager.h"
#include "book_model.h"
#include "financial_statement.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Book book_;  // TODO: consider using friend class then put `book_` to private?

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

    Ui::MainWindow *ui;

    AccountManager     *account_manager_;
    FinancialStatement *financial_statement_;

    BookModel book_model_;

    // Filter components:
    QDateEdit* start_date_;
    QDateEdit* end_date_;
    QLineEdit* description_;
    QVector<QComboBox*> category_combo_boxes_;
    QVector<QComboBox*> name_combo_boxes_;
};

#endif // MAIN_WINDOW_H
