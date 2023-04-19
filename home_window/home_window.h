#ifndef HOME_WINDOW_H
#define HOME_WINDOW_H

#include <QMainWindow>
#include <QDateEdit>

#include "account_manager/account_manager.h"
#include "book_model.h"
#include "financial_statement.h"
#include "household_manager/household_manager.h"

namespace Ui {
class HomeWindow;
}

class HomeWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit HomeWindow(QWidget *parent = nullptr);
      ~HomeWindow();

  public slots:
    void refreshTable();  // Show all filtered transactions.
    void setCategoryComboBox();

  protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void closeEvent(QCloseEvent *event) override;

  private slots:
    void onActionAddTransactionTriggered();
    void onActionAccountManagerTriggered();
    void onActionFinancialStatementTriggered();
    void onActionInvestmentAnalysisTriggered();
    void onActionTransactionValidationTriggered();
    void onActionHouseholdManagerTriggered();

    void onPushButtonDeleteClicked();
    void onPushButtonMergeClicked();

    void accountCategoryChanged(const Account::Type& table_type, const QString& category, QComboBox* name_combo_box);
    void onTableViewDoubleClicked(const QModelIndex &index);

private:
    void resizeColumns();

    Ui::HomeWindow* ui;

    QSharedPointer<AccountManager>   account_manager_;
    QSharedPointer<HouseholdManager> household_manager_;

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
    friend class InvestmentAnalysis;
    friend class HouseholdManager;
};

#endif // HOME_WINDOW_H
