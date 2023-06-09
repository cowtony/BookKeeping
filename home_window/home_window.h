#ifndef HOME_WINDOW_H
#define HOME_WINDOW_H

#include <QMainWindow>
#include <QDateEdit>
#include <QTableView>

#include "account_manager/account_manager.h"
#include "transactions_model.h"
#include "household_manager/household_manager.h"

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
    void onActionLoginTriggered();
    void onActionLogoutTriggered();

    void onPushButtonMergeClicked();
    void onPushButtonDeleteClicked();

    void accountCategoryChanged(const Account::Type& table_type, const QString& category, QComboBox* name_combo_box);
    void onTableViewDoubleClicked(const QModelIndex& index);

  private:
    void resizeTableView(QTableView* table_view);

    Ui::HomeWindow* ui;

    QSharedPointer<AccountManager>   account_manager_;
    QSharedPointer<HouseholdManager> household_manager_;

    TransactionsModel transactions_model_;

    // Filter components:
    QVector<QComboBox*> category_combo_boxes_;
    QVector<QComboBox*> name_combo_boxes_;
};

#endif // HOME_WINDOW_H
