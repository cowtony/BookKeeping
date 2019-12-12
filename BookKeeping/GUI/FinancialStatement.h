#ifndef FINANCIALSTATEMENT_H
#define FINANCIALSTATEMENT_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "Book.h"

namespace Ui {
class FinancialStatement;
}

class FinancialStatement : public QMainWindow
{
    Q_OBJECT

public:
    explicit FinancialStatement(QWidget *parent = nullptr);
    ~FinancialStatement();

public slots:
    void on_pushButton_Query_clicked();

private slots:
    void on_treeWidget_itemCollapsed(QTreeWidgetItem *item);
    void on_treeWidget_itemExpanded(QTreeWidgetItem *item);
    void on_radioButton_all_clicked();
    void on_radioButton_1_clicked();
    void on_radioButton_2_clicked();
    void on_pushButtonExport_clicked();
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::FinancialStatement *ui;

    void setMoney(QTreeWidgetItem *item, const int &p_column, const Money &p_money);
    void display();
    QTreeWidgetItem* getAccountItem(const Account& account, const bool& create = false); // Get or create account item.

    static QFont m_financialStatementFont;
    static QFont m_tableSumFont;
    static QFont m_categorySumFont;
    int m_viewSelection;  // -1: all, 0: person 0, 1: person 1

    QList<FinancialStat> m_records;
    int m_startIndex = 0;
    int m_length = 6;
};

#endif // FINANCIALSTATEMENT_H
