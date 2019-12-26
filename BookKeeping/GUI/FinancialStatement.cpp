#include "FinancialStatement.h"
#include "ui_FinancialStatement.h"
#include <QtMath>
#include <QClipboard>
#include "Book.h"
#include "BarChart.h"

QFont FinancialStatement::m_financialStatementFont = QFont("Times New Roman", 14, 1, false);
QFont FinancialStatement::m_tableSumFont = QFont("Times New Roman", 12, 1, false);
QFont FinancialStatement::m_categorySumFont;

FinancialStatement::FinancialStatement(QWidget *parent) : QMainWindow(parent), ui(new Ui::FinancialStatement) {
  ui->setupUi(this);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  m_viewSelection = -1;
  ui->radioButton_all->setChecked(true);

  m_financialStatementFont.setBold(true);
  m_financialStatementFont.setUnderline(true);
  m_tableSumFont.setBold(true);
  m_tableSumFont.setUnderline(true);
  m_categorySumFont.setBold(true);
}

FinancialStatement::~FinancialStatement() {
  delete ui;
}

void FinancialStatement::on_pushButton_Query_clicked() {
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_records = g_book.getSummaryByMonth(ui->dateTimeEdit->dateTime());
  QApplication::restoreOverrideCursor();

  display();
}

void FinancialStatement::setMoney(QTreeWidgetItem* item, const int& column, const Money& money) {
  if (item == nullptr)
    return;

  // Get the last date of the month.
  QDate date = QDate::fromString(ui->treeWidget->headerItem()->text(column), "yyyy-MM");
  date = date.addMonths(1);
  date = date.addDays(-1);
  const Money difference = money - Money(date, item->text(column));

  item->setText(column, money.toString());
  if (money.m_amount < 0) {
    item->setTextColor(column, QColor(Qt::red));
  } else {
    item->setTextColor(column, QColor(Qt::black));
  }

  // Recursivly update parent sum:
  if (item->parent() != nullptr) {
    const Money parentMoney(date, item->parent()->text(column));

    if (item->text(0) == "Expense" || item->text(0) == "Liability") {
      setMoney(item->parent(), column, parentMoney - difference);
    } else if (item->text(0) == "Equity") {
      // do nothing.
    } else {
      setMoney(item->parent(), column, parentMoney + difference);
    }
  }
}

void FinancialStatement::setFont(const int& column, QTreeWidgetItem* item, const int& depth) {
  switch (depth) {
    case 0:
      item->setFont(column, m_financialStatementFont);
      break;
    case 1:
      item->setFont(column, m_tableSumFont);
      break;
    case 2:
      item->setFont(column, m_categorySumFont);
      break;
    case 3:
      break;
    default:
      break;
  }
  if (column > 0) {
    item->setTextAlignment(column, Qt::AlignRight);
  }

  for (int i = 0; i < item->childCount(); i++) {
    setFont(column, item->child(i), depth + 1);
  }
}

void FinancialStatement::display() {
  ui->treeWidget->clear();
  ui->treeWidget->header()->setDefaultAlignment(Qt::AlignCenter);
  ui->treeWidget->setColumnCount(1);
  ui->treeWidget->setHeaderLabels({"Name"});

  on_pushButtonShowMore_clicked();  // Show the most recent month only.
  ui->treeWidget->expandToDepth(1);
}

QTreeWidgetItem* FinancialStatement::getAccountItem(const Account& account, const bool& create) {
  // Find or create same financial statement item:
  QTreeWidgetItem *statementItem = nullptr;
  for (int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
    if (ui->treeWidget->topLevelItem(i)->text(0) == account.getFinancialStatementName()) {
      statementItem = ui->treeWidget->topLevelItem(i);
      break;
    }
  }
  if (statementItem == nullptr) {
    if (create) {
      statementItem = new QTreeWidgetItem(ui->treeWidget, {account.getFinancialStatementName()});
      statementItem->setFont(0, m_financialStatementFont);
    } else {
      return nullptr;
    }
  }

  // Find or create same table:
  QTreeWidgetItem *tableItem = nullptr;
  for (int i = 0; i < statementItem->childCount(); i++) {
    if (statementItem->child(i)->text(0) == account.getTableName()) {
      tableItem = statementItem->child(i);
      break;
    }
  }
  if (tableItem == nullptr) {
    if (create) {
      tableItem = new QTreeWidgetItem(statementItem, {account.getTableName()});
      tableItem->setFont(0, m_tableSumFont);
    } else {
      return nullptr;
    }
  }

  // Find or create same category:
  QTreeWidgetItem *categoryItem = nullptr;
  for (int i = 0; i < tableItem->childCount(); i++) {
    if (tableItem->child(i)->text(0) == account.m_category) {
      categoryItem = tableItem->child(i);
      break;
    }
  }
  if (categoryItem == nullptr) {
    if (create) {
      categoryItem = new QTreeWidgetItem(tableItem, {account.m_category});
      categoryItem->setFont(0, m_categorySumFont);
    } else {
      return nullptr;
    }
  }

  // Find or create same account:
  QTreeWidgetItem *accountItem = nullptr;
  for (int i = 0; i < categoryItem->childCount(); i++) {
    if (categoryItem->child(i)->text(0) == account.m_name) {
      accountItem = categoryItem->child(i);
      break;
    }
  }
  if (accountItem == nullptr) {
    if (create) {
      accountItem = new QTreeWidgetItem(categoryItem, {account.m_name});
    } else {
      return nullptr;
    }
  }

  return accountItem;
}

void FinancialStatement::on_treeWidget_itemCollapsed(QTreeWidgetItem *item) {
  for (int i = 0; i < ui->treeWidget->columnCount(); i++) {
    ui->treeWidget->resizeColumnToContents(i);
  }
}

void FinancialStatement::on_treeWidget_itemExpanded(QTreeWidgetItem *item)
{
    for (int i = 0; i < ui->treeWidget->columnCount(); i++)
        ui->treeWidget->resizeColumnToContents(i);
}

void FinancialStatement::on_radioButton_all_clicked()
{
    m_viewSelection = -1;
    display();
}

void FinancialStatement::on_radioButton_1_clicked()
{
    m_viewSelection = 0;
    display();
}

void FinancialStatement::on_radioButton_2_clicked()
{
    m_viewSelection = 1;
    display();
}

void FinancialStatement::on_pushButtonExport_clicked()
{
    QStringList rows;

    QStringList cells;
    for (int i = 0; i <= m_records.size(); i++)
    {
        cells << ui->treeWidget->headerItem()->text(i);
    }
    rows << cells.join('\t');

    for (QTreeWidgetItem *item : ui->treeWidget->findItems("", Qt::MatchContains | Qt::MatchRecursive, 0))
    {
        QStringList cells;

        QStringList names;
        QTreeWidgetItem *tempItem = item;
        while (tempItem != nullptr)
        {
            names.push_front(tempItem->text(0));
            tempItem = tempItem->parent();
        }
        cells << names.join("::");

        for (int i = 1; i <= m_records.size(); i++)
        {
            cells << item->text(i);
        }
        rows << cells.join('\t');
    }

    QApplication::clipboard()->setText(rows.join('\n'));
}

void FinancialStatement::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QStringList pathway;
    QTreeWidgetItem *tempItem = item;
    while (tempItem != nullptr)
    {
        pathway.push_front(tempItem->text(0));
        tempItem = tempItem->parent();
    }

    switch (pathway.size()) {
    case 1: // click on financial statement
    case 2: // click on table
    case 3: // click on category
    {
        BarChart *barChart = new BarChart(this);
        barChart->setAttribute(Qt::WA_DeleteOnClose);
        barChart->setTitle(pathway.join("::"));

        QStringList xAxis;
        QList<qreal> sum;
        for (int col = 1; col < ui->treeWidget->columnCount(); col++)
        {
            xAxis.push_front(ui->treeWidget->headerItem()->text(col));
            sum.push_front(Money(QDate(), item->text(col)).m_amount);
        }
        barChart->setAxisX(xAxis);
        barChart->addLine(item->text(0), sum);

        for (int index = 0; index < item->childCount(); index++)
        {
            QList<qreal> data;
            for (int j = 0; j < xAxis.size(); j++)
            {
                Money money(QDate::fromString(xAxis.at(j)), item->child(index)->text(j + 1));
                money.changeCurrency(USD);
                if (item->text(0) == "Balance Sheet"    and item->child(index)->text(0) == "Equity")
                    continue;
                if ((item->text(0) == "Income Statement" and item->child(index)->text(0) == "Expense") or
                    (item->text(0) == "Balance Sheet"    and item->child(index)->text(0) == "Liability"))
                    data.push_front(-money.m_amount);
                else
                    data.push_front(money.m_amount);
            }
            barChart->addBarSetToStackedBarSeries(item->child(index)->text(0), data);
        }

        barChart->showStackedBarChart();
        break;
    }
    case 4: // click on account
    {
        BarChart *barChart = new BarChart(this);
        barChart->setAttribute(Qt::WA_DeleteOnClose);
        barChart->setTitle(pathway.join("::"));

        QStringList xAxis;
        for (int i = 0; i < m_records.size(); i++)
            xAxis.push_front(m_records.at(i).m_description);
        barChart->setAxisX(xAxis);

        QList<qreal> yAxis1;
        QList<qreal> yAxis2;
        Account account(pathway.at(1), pathway.at(2), pathway.at(3));
        for (int i = 0; i < m_records.size(); i++)
        {
            yAxis1.push_front(m_records.at(i).getMoneyArray(account).getMoney(0).m_amount);
            yAxis2.push_front(m_records.at(i).getMoneyArray(account).getMoney(1).m_amount);
        }
        barChart->addBarSet("Person 1", yAxis1);
        barChart->addBarSet("Person 2", yAxis2);

        barChart->show();
        break;
    }
    }
}

void FinancialStatement::on_pushButtonShowMore_clicked() {
  int index = ui->treeWidget->columnCount() - 1;
  if (index >= m_records.size()) {
    return;
  }

  ui->treeWidget->setColumnCount(index + 2);
  ui->treeWidget->headerItem()->setText(index + 1, m_records.at(index).m_description);

  for (const Account& account : m_records.at(index).getAccounts()) {
    MoneyArray moneyArray = m_records.at(index).getMoneyArray(account);
    QTreeWidgetItem* accountItem = getAccountItem(account, true);
    if (m_viewSelection == -1) {
      setMoney(accountItem, index + 1, moneyArray.sum());
    } else {
      setMoney(accountItem, index + 1, moneyArray.getMoney(m_viewSelection));
    }
  }

  // Must setFont after getAccountItem() because some item may not be created before that.
  setFont(index + 1, ui->treeWidget->invisibleRootItem());

  ui->treeWidget->resizeColumnToContents(index + 1);
}

