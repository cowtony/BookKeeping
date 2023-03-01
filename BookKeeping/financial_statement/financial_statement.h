#ifndef FINANCIAL_STATEMENT_H
#define FINANCIAL_STATEMENT_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "book.h"

namespace Ui {
class FinancialStatement;
}

class FinancialStatement : public QMainWindow {
  Q_OBJECT

public:
  explicit FinancialStatement(Book& book, QWidget *parent = nullptr);
  ~FinancialStatement();

public slots:
  void on_pushButton_Query_clicked();

private slots:
  void on_treeWidget_itemCollapsed(QTreeWidgetItem *item);
  void on_treeWidget_itemExpanded(QTreeWidgetItem *item);
  void on_radioButton_all_clicked();
  void on_radioButton_1_clicked();
  void on_radioButton_2_clicked();
  void on_radioButton_3_clicked();
  void on_pushButtonExport_clicked();
  void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

  void on_pushButtonShowMore_clicked();

  void on_pushButton_clicked();

private:
  Ui::FinancialStatement *ui;
  Book& book_;

  void setMoney(QTreeWidgetItem* item, int column, const Money& money);
  void setFont(int column, QTreeWidgetItem* item, int depth = -1);
  void display();
  QTreeWidgetItem* getAccountItem(const Account& account, bool create = false); // Get or create account item.

  static QFont m_financialStatementFont;
  static QFont m_tableSumFont;
  static QFont m_categorySumFont;
  int m_viewSelection;  // -1: all, 0: person 0, 1: person 1

  QList<FinancialStat> m_records;
};

#endif // FINANCIAL_STATEMENT_H
