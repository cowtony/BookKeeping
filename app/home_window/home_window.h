#ifndef HOME_WINDOW_H
#define HOME_WINDOW_H

#include <QMainWindow>
#include <QDateEdit>
#include <QTableView>

#include "account_manager/account_manager.h"
#include "household_manager/household_manager.h"
#include "financial_statement/financial_statement.h"
#include "transactions_model.h"

namespace Ui {
class HomeWindow;
}

class HomeWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit HomeWindow(QWidget *parent = nullptr);
      ~HomeWindow();

    Book book;
    int user_id;
    AccountManager     account_manager;
    HouseholdManager   household_manager;
    FinancialStatement financial_statement;

public slots:
    void refreshTable();  // Show all filtered transactions.
    void setCategoryComboBox();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void closeEvent(QCloseEvent *event) override;

private slots:
    void onActionAddTransactionTriggered();
    void onActionAccountManagerTriggered()     { account_manager.show(); }
    void onActionHouseholdManagerTriggered()   { household_manager.show(); }
    void onActionFinancialStatementTriggered() { financial_statement.show(); }
    void onActionInvestmentAnalysisTriggered();
    void onActionTransactionValidationTriggered();
    void onActionLoginTriggered();
    void onActionLogoutTriggered();
    void onPushButtonMergeClicked();
    void onPushButtonDeleteClicked();

    void accountCategoryChanged(const Account::Type& table_type, const QString& new_category_name, QComboBox* name_combo_box);
    void onTableViewDoubleClicked(const QModelIndex& index);

private:
    void resizeTableView(QTableView* table_view);

    Ui::HomeWindow* ui;

    TransactionsModel transactions_model_;

    // Filter components:
    QVector<QComboBox*> category_combo_boxes_;
    QVector<QComboBox*> name_combo_boxes_;
};

#endif // HOME_WINDOW_H
