#include "financial_statement.h"
#include "ui_financial_statement.h"

#include <QtMath>
#include <QClipboard>

#include "home_window/home_window.h"
#include "bar_chart.h"

QFont FinancialStatement::m_financialStatementFont = QFont("Times New Roman", 14, 1, false);
QFont FinancialStatement::m_tableSumFont = QFont("Times New Roman", 12, 1, false);
QFont FinancialStatement::m_categorySumFont;

FinancialStatement::FinancialStatement(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::FinancialStatement),
      book_(static_cast<HomeWindow*>(parent)->book),
      user_id_(static_cast<HomeWindow*>(parent)->user_id) {

    ui->setupUi(this);

    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    ui->comboBoxHousehold->addItems(QStringList() << "All" << book_.getHouseholds(user_id_));

    m_financialStatementFont.setBold(true);
    m_financialStatementFont.setUnderline(true);
    m_tableSumFont.setBold(true);
    m_tableSumFont.setUnderline(true);
    m_categorySumFont.setBold(true);

    connect(ui->treeWidget, &QTreeWidget::itemCollapsed, this, &FinancialStatement::onTreeWidgetItemCollapsed);
    connect(ui->treeWidget, &QTreeWidget::itemExpanded,  this, &FinancialStatement::onTreeWidgetItemExpanded);
    connect(ui->treeWidget, &QTreeWidget::itemClicked,   this, &FinancialStatement::onTreeWidgetItemClicked);
    connect(ui->pushButtonExport,   &QPushButton::clicked, this, &FinancialStatement::onPushButtonExportClicked);
    connect(ui->pushButtonShowMore, &QPushButton::clicked, this, &FinancialStatement::onPushButtonShowMoreClicked);
    connect(ui->pushButtonShowAll,  &QPushButton::clicked, this, &FinancialStatement::onPushButtonShowAllClicked);
}

void FinancialStatement::on_pushButton_Query_clicked() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    monthly_stats_ = getSummaryByMonth(ui->dateTimeEdit->dateTime());
    QApplication::restoreOverrideCursor();
    display();
}

void FinancialStatement::setMoney(QTreeWidgetItem* item, int column, const Money& money) {
    if (item == nullptr) {
        return;
    }

  // Get the last date of the month.
    QDate date = QDate::fromString(ui->treeWidget->headerItem()->text(column), "yyyy-MM");
  date = date.addMonths(1);
  date = date.addDays(-1);
  const Money difference = money - Money(date, item->text(column));

  item->setText(column, money);
  if (money.amount_ < 0) {
    item->setForeground(column, Qt::red);
  } else {
    item->setForeground(column, QColor(Qt::black));
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

void FinancialStatement::setFont(int column, QTreeWidgetItem* item, int depth) {
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
    for (int i = 0; i < 10; i++) {  // Show the most recent 10 month.
        onPushButtonShowMoreClicked();
    }
    ui->treeWidget->expandToDepth(1);
}

QTreeWidgetItem* FinancialStatement::getAccountItem(const Account& account, bool create) {
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
    if (statementItem->child(i)->text(0) == account.typeName()) {
      tableItem = statementItem->child(i);
      break;
    }
  }
  if (tableItem == nullptr) {
    if (create) {
      tableItem = new QTreeWidgetItem(statementItem, {account.typeName()});
      tableItem->setFont(0, m_tableSumFont);
    } else {
      return nullptr;
    }
  }

  // Find or create same category:
  QTreeWidgetItem *categoryItem = nullptr;
  for (int i = 0; i < tableItem->childCount(); i++) {
    if (tableItem->child(i)->text(0) == account.categoryName()) {
      categoryItem = tableItem->child(i);
      break;
    }
  }
  if (categoryItem == nullptr) {
    if (create) {
      categoryItem = new QTreeWidgetItem(tableItem, {account.categoryName()});
      categoryItem->setFont(0, m_categorySumFont);
    } else {
      return nullptr;
    }
  }

  // Find or create same account:
  QTreeWidgetItem *accountItem = nullptr;
  for (int i = 0; i < categoryItem->childCount(); i++) {
    if (categoryItem->child(i)->text(0) == account.accountName()) {
      accountItem = categoryItem->child(i);
      break;
    }
  }
  if (accountItem == nullptr) {
    if (create) {
      accountItem = new QTreeWidgetItem(categoryItem, {account.accountName()});
    } else {
      return nullptr;
    }
  }

  return accountItem;
}

void FinancialStatement::onTreeWidgetItemCollapsed(QTreeWidgetItem* /* item */) {
  for (int i = 0; i < ui->treeWidget->columnCount(); i++) {
    ui->treeWidget->resizeColumnToContents(i);
  }
}

void FinancialStatement::onTreeWidgetItemExpanded(QTreeWidgetItem* /* item */) {
  for (int i = 0; i < ui->treeWidget->columnCount(); i++) {
    ui->treeWidget->resizeColumnToContents(i);
  }
}

void FinancialStatement::onPushButtonExportClicked()
{
    QStringList rows;

    QStringList cells;
    for (int i = 0; i <= monthly_stats_.size(); i++)
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

        for (int i = 1; i <= monthly_stats_.size(); i++) {
            cells << item->text(i);
        }
        rows << cells.join('\t');
    }

    QApplication::clipboard()->setText(rows.join('\n'));
}

void FinancialStatement::onTreeWidgetItemClicked(QTreeWidgetItem* item, int /* column */) {
    QStringList pathway;
    QTreeWidgetItem *tempItem = item;
    while (tempItem != nullptr) {
        pathway.push_front(tempItem->text(0));
        tempItem = tempItem->parent();
    }

    switch (pathway.size()) {
        case 1: // click on financial statement
        case 2: // click on table
        case 3: { // click on category
            BarChart *barChart = new BarChart(this);
            barChart->setAttribute(Qt::WA_DeleteOnClose);
            barChart->setTitle(pathway.join("::"));

            QStringList xAxis;
            QList<qreal> sum;
            for (int col = 1; col < ui->treeWidget->columnCount(); col++) {
                xAxis.push_front(ui->treeWidget->headerItem()->text(col));
                sum.push_front(Money(QDate(), item->text(col)).amount_);
            }
            barChart->setAxisX(xAxis);
            barChart->addLine(item->text(0), sum);

            for (int index = 0; index < item->childCount(); index++) {
                QList<qreal> data;
                for (int j = 0; j < xAxis.size(); j++) {
                    Money money(QDate::fromString(xAxis.at(j)), item->child(index)->text(j + 1));
                    money.changeCurrency(Currency::USD);
                    if (item->text(0) == "Balance Sheet"    and item->child(index)->text(0) == "Equity") {
                        continue;
                    }
                    if ((item->text(0) == "Income Statement" and item->child(index)->text(0) == "Expense") or
                        (item->text(0) == "Balance Sheet"    and item->child(index)->text(0) == "Liability")) {
                        data.push_front(-money.amount_);
                    } else {
                        data.push_front(money.amount_);
                    }
                }
                barChart->addBarSetToStackedBarSeries(item->child(index)->text(0), data);
            }

            barChart->showStackedBarChart();
            break;
        }
        case 4: {// click on account
            BarChart *barChart = new BarChart(this);
            barChart->setAttribute(Qt::WA_DeleteOnClose);
            barChart->setTitle(pathway.join("::"));

            QStringList x_axis(monthly_stats_.size());
            for (int i = 0; i < monthly_stats_.size(); i++) {
                x_axis[monthly_stats_.size() - 1 - i] = monthly_stats_.at(i).description;
            }
            barChart->setAxisX(x_axis);

            QHash<QString, QList<qreal>> y_axes;
            for (int i = 0; i < monthly_stats_.size(); i++) {
                HouseholdMoney household_money = monthly_stats_.at(i).getHouseholdMoney(Account::kAccountTypeName.key(pathway.at(1)), pathway.at(2), pathway.at(3));
                for (const auto& [household_name, money] : household_money.data().asKeyValueRange()) {
                    if (!y_axes.contains(household_name)) {
                        y_axes[household_name] = QList<qreal>(monthly_stats_.size(), 0.0);
                    }
                    y_axes[household_name][monthly_stats_.size() - 1 - i] = money.amount_;
                }
            }
            for (const auto& [household_name, y_axis] : y_axes.asKeyValueRange()) {
                barChart->addBarSet(household_name, y_axis);
            }

            barChart->show();
            break;
        }
    }
}

void FinancialStatement::onPushButtonShowMoreClicked() {
    int index = ui->treeWidget->columnCount() - 1;
    if (index >= monthly_stats_.size()) {
        return;
    }

    ui->treeWidget->setColumnCount(index + 2);
    ui->treeWidget->headerItem()->setText(index + 1, monthly_stats_.at(index).description);

    for (const auto& [account, household_money] : monthly_stats_.at(index).getAccounts()) {
        QTreeWidgetItem* accountItem = getAccountItem(*account, true);
        if (ui->comboBoxHousehold->currentText() == "All") {
            setMoney(accountItem, index + 1, household_money.sum());
        } else {
            setMoney(accountItem, index + 1, household_money.data().value(ui->comboBoxHousehold->currentText()));
        }
    }

    // Must setFont after getAccountItem() because some item may not be created before that.
    setFont(index + 1, ui->treeWidget->invisibleRootItem());

    ui->treeWidget->resizeColumnToContents(index + 1);
}

void FinancialStatement::onPushButtonShowAllClicked() {
    while (ui->treeWidget->columnCount() <= monthly_stats_.size()) {
        onPushButtonShowMoreClicked();
    }
}

QList<FinancialStat> FinancialStatement::getSummaryByMonth(const QDateTime &end_date_time) const {
    QList<FinancialStat> result;
    FinancialStat monthly_stat;
    QDate first_transaction_date = book_.getFirstTransactionDateTime().date();
    QDate month = QDate(first_transaction_date.year(), first_transaction_date.month(), 1);
    QElapsedTimer timer;
    timer.start();
    for (const Transaction& transaction : book_.queryTransactions(user_id_, TransactionFilter().toTime(end_date_time).orderByAscending())) {
        // Use `while` instead of `if` in case there was no transaction for successive months.
        while (transaction.date_time.date() >= month.addMonths(1)) {
            monthly_stat.description = month.toString("yyyy-MM");
            monthly_stat.cumulateRetainedEarning();
            result.push_front(monthly_stat);

            month = month.addMonths(1);
            monthly_stat.clear(Account::Revenue);
            monthly_stat.clear(Account::Expense);
        }

        monthly_stat.cumulateCurrencyError(transaction.date_time);  // This is the most time consuming part because this loop through all accounts in monthly_stat, which is ALL accounts in db.
        monthly_stat.cumulateTransaction(transaction);
    }

    monthly_stat.description = month.toString("yyyy-MM");
    monthly_stat.cumulateRetainedEarning();
    result.push_front(monthly_stat);
    qDebug() << "Total Time used: " << timer.elapsed() / 1000.0 << "seconds";
    return result;
}

void FinancialStatement::on_comboBoxHousehold_currentIndexChanged(int /* index */) {
    display();
}

