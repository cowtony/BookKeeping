#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateEdit>
#include "AccountManager.h"
#include "FinancialStatement.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
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

    void on_pushButtonDeleteTransactions_clicked();
    void accountCategoryChanged(const Account::TableType &tableType, const QString& category);
    void displayTransactions();
    void on_tableWidget_transactions_cellDoubleClicked(int row, int column);
    void setCategoryComboBox();

    void on_pushButton_MergeTransaction_clicked();





private:
    Ui::MainWindow *ui;

    AccountManager     *accountManager;
    FinancialStatement *financialStatement;

    QDateEdit *startDate;
    QDateEdit *endDate;

    const QMap<int, Account::TableType> TableIndex;

    QMap<QDateTime, QString> getSelectedTransactionInfos() const;
};

#endif // MAINWINDOW_H
