#ifndef FINANCIAL_STATEMENT_H
#define FINANCIAL_STATEMENT_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "book/book.h"

namespace Ui {
class FinancialStatement;
}

class FinancialStatement : public QMainWindow {
    Q_OBJECT

public:
    explicit FinancialStatement(QWidget *parent);

public slots:
    void on_pushButton_Query_clicked();
    QPair<QDate, FinancialStat> getStartStateFor(QDate query_date);

private slots:
    void onTreeWidgetItemCollapsed(QTreeWidgetItem *item);
    void onTreeWidgetItemExpanded(QTreeWidgetItem *item);
    void onTreeWidgetItemClicked(QTreeWidgetItem *item, int column);
    void on_comboBoxHousehold_currentIndexChanged(int index);
    void onPushButtonExportClicked();
    void onPushButtonShowMoreClicked();
    void onPushButtonShowAllClicked();

private:
    void setMoney(QTreeWidgetItem* item, int column, const Money& money);
    void setFont(int column, QTreeWidgetItem* item, int depth = -1);
    void refreshTableWidget();
    QTreeWidgetItem* getAccountItem(const Account& account, bool create = false); // Get or create account item.
    void getSummaryByMonth(const QDateTime& p_endDateTime = QDateTime(QDate(2100, 12, 31), QTime(0, 0, 0)));

    static QFont kFinancialStatementFont;
    static QFont kTableSumFont;
    static QFont kCategorySumFont;

    Ui::FinancialStatement* ui;

    Book& book_;
    int& user_id_;

    int columns_to_display_ = 1;

    QList<FinancialStat> monthly_stats_;
};

#endif // FINANCIAL_STATEMENT_H
