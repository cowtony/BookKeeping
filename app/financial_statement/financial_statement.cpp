#include "financial_statement.h"
#include "ui_financial_statement.h"

#include <QtMath>
#include <QClipboard>

#include "home_window/home_window.h"
#include "bar_chart.h"

QFont FinancialStatement::kFinancialStatementFont = QFont("Times New Roman", 14, 1, false);
QFont FinancialStatement::kTableSumFont = QFont("Times New Roman", 12, 1, false);
QFont FinancialStatement::kCategorySumFont;

FinancialStatement::FinancialStatement(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::FinancialStatement),
      book_(static_cast<HomeWindow*>(parent)->book),
      user_id_(static_cast<HomeWindow*>(parent)->user_id) {

    ui->setupUi(this);

    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

    ui->comboBoxHousehold->addItems(QStringList() << "All" << book_.getHouseholds(user_id_));

    kFinancialStatementFont.setBold(true);
    kFinancialStatementFont.setUnderline(true);
    kTableSumFont.setBold(true);
    kTableSumFont.setUnderline(true);
    kCategorySumFont.setBold(true);

    connect(ui->treeWidget, &QTreeWidget::itemCollapsed, this, &FinancialStatement::onTreeWidgetItemCollapsed);
    connect(ui->treeWidget, &QTreeWidget::itemExpanded,  this, &FinancialStatement::onTreeWidgetItemExpanded);
    connect(ui->treeWidget, &QTreeWidget::itemClicked,   this, &FinancialStatement::onTreeWidgetItemClicked);
    connect(ui->pushButtonExport,   &QPushButton::clicked, this, &FinancialStatement::onPushButtonExportClicked);
    connect(ui->pushButtonShowMore, &QPushButton::clicked, this, &FinancialStatement::onPushButtonShowMoreClicked);
    connect(ui->pushButtonShowMore, &QPushButton::clicked, this, [this](){ columns_to_display_++; });
    connect(ui->pushButtonShowAll,  &QPushButton::clicked, this, &FinancialStatement::onPushButtonShowAllClicked);
}

void FinancialStatement::on_pushButton_Query_clicked() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QElapsedTimer timer;
    timer.start();
    getSummaryByMonth(ui->dateTimeEdit->dateTime());
    QApplication::restoreOverrideCursor();
    qDebug() << "Total time used:" << timer.elapsed() / 1000.0 << "seconds";
    refreshTableWidget();
}

void FinancialStatement::setMoney(QTreeWidgetItem* item, int column, const Money& money) {
    if (item == nullptr) {
        return;
    }

    const Money difference = money - Money(money.utcDate, item->text(column));

    item->setText(column, money);
    if (money.amount_ < 0) {
        item->setForeground(column, Qt::red);
    } else {
        item->setForeground(column, QColor(Qt::black));
    }

    // Recursivly update parent sum:
    if (item->parent() != nullptr) {
        const Money parentMoney(money.utcDate, item->parent()->text(column));

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
        item->setFont(column, kFinancialStatementFont);
        break;
    case 1:
        item->setFont(column, kTableSumFont);
        break;
    case 2:
        item->setFont(column, kCategorySumFont);
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

void FinancialStatement::refreshTableWidget() {
    ui->treeWidget->clear();
    ui->treeWidget->header()->setDefaultAlignment(Qt::AlignCenter);
    ui->treeWidget->setColumnCount(1);
    ui->treeWidget->setHeaderLabels({"Name"});
//    columns_to_display_ = qMin(columns_to_display_, 12);  // Cap the column number otherwise it's taking too much time.
    for (int i = 0; i < columns_to_display_; i++) {
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
            statementItem->setFont(0, kFinancialStatementFont);
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
            tableItem->setFont(0, kTableSumFont);
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
            categoryItem->setFont(0, kCategorySumFont);
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

void FinancialStatement::onPushButtonExportClicked() {
    QStringList rows;

    QStringList cells;
    for (int i = 0; i < ui->treeWidget->columnCount(); i++) {
        cells << ui->treeWidget->headerItem()->text(i);
    }
    rows << cells.join('\t');

    for (QTreeWidgetItem *item : ui->treeWidget->findItems("", Qt::MatchContains | Qt::MatchRecursive, 0)) {
        QStringList cells;

        QStringList names;
        QTreeWidgetItem *tempItem = item;
        while (tempItem != nullptr) {
            names.push_front(tempItem->text(0));
            tempItem = tempItem->parent();
        }
        cells << names.join("::");

        for (int i = 1; i < ui->treeWidget->columnCount(); i++) {
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
            BarChart *bar_chart = new BarChart(this);
            bar_chart->setAttribute(Qt::WA_DeleteOnClose);
            bar_chart->setTitle(pathway.join("::"));

            QStringList xAxis;
            QList<qreal> sum;
            for (int col = 1; col < ui->treeWidget->columnCount(); col++) {
                xAxis.push_front(ui->treeWidget->headerItem()->text(col));
                sum.push_front(Money(QDate(), item->text(col)).amount_);
            }
            bar_chart->setAxisX(xAxis);

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
                bar_chart->addBarSetToStackedBarSeries(item->child(index)->text(0), data);
            }
            bar_chart->addStackedBarSeries();
            bar_chart->addLine(item->text(0), sum);
            bar_chart->show();
            break;
        }
        case 4: {// click on account
            BarChart *bar_chart = new BarChart(this);
            bar_chart->setAttribute(Qt::WA_DeleteOnClose);
            bar_chart->setTitle(pathway.join("::"));

            QStringList x_axis(monthly_stats_.size());
            for (int i = 0; i < monthly_stats_.size(); i++) {
                x_axis[i] = monthly_stats_.at(i).description;
            }
            bar_chart->setAxisX(x_axis);

            QHash<QString, QList<qreal>> y_axes;
            for (int i = 0; i < monthly_stats_.size(); i++) {
                HouseholdMoney household_money = monthly_stats_.at(i).getHouseholdMoney(Account::kAccountTypeName.key(pathway.at(1)), pathway.at(2), pathway.at(3));
                for (const auto& [household_name, money] : household_money.data().asKeyValueRange()) {
                    if (!y_axes.contains(household_name)) {
                        y_axes[household_name] = QList<qreal>(monthly_stats_.size(), 0.0);
                    }
                    y_axes[household_name][i] = money.amount_;
                }
            }
            bar_chart->addBarSeries();
            for (const auto& [household_name, y_axis] : y_axes.asKeyValueRange()) {
                bar_chart->addBarSet(household_name, y_axis);
                bar_chart->addLine(household_name, y_axis);
            }
            bar_chart->show();
            break;
        }
    }
}

void FinancialStatement::onPushButtonShowMoreClicked() {
    int column_count = ui->treeWidget->columnCount();
    if (column_count > monthly_stats_.size()) {
        return;
    }
    const FinancialStat& new_month = monthly_stats_.at(monthly_stats_.size() - column_count);

    ui->treeWidget->setColumnCount(column_count + 1);
    ui->treeWidget->headerItem()->setText(column_count, new_month.description);

    for (const auto& [account, household_money] : new_month.getAccounts()) {
        QTreeWidgetItem* accountItem = getAccountItem(*account, true);
        Money money = ui->comboBoxHousehold->currentText() == "All" ?
                          household_money.sum() :
                          household_money.data().value(ui->comboBoxHousehold->currentText());
        money.utcDate = new_month.utcDate_;
        setMoney(accountItem, column_count, money);
    }

    // Must setFont after getAccountItem() because some item may not be created before that.
    setFont(column_count, ui->treeWidget->invisibleRootItem());

    ui->treeWidget->resizeColumnToContents(column_count);
}

void FinancialStatement::onPushButtonShowAllClicked() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QElapsedTimer timer;
    timer.start();
    while (ui->treeWidget->columnCount() <= monthly_stats_.size()) {
        onPushButtonShowMoreClicked();
    }
    QApplication::restoreOverrideCursor();
    qDebug() << "Total time used:" << timer.elapsed() / 1000.0 << "seconds";
}

void FinancialStatement::getSummaryByMonth(const QDateTime &end_date_time) {
    auto [month, monthly_stat] = getStartStateFor(end_date_time.toUTC().date());

    for (const Transaction& transaction : book_.queryTransactions(user_id_, TransactionFilter().startTime(QDateTime(month, QTime(0, 0, 0), QTimeZone::utc()))
                                                                                               .endTime(end_date_time)
                                                                                               .orderByAscending())) {
        // Use `while` instead of `if` in case there was no transaction for successive months.
        while (transaction.date_time.toUTC().date() >= month.addMonths(1)) {
            monthly_stat.description = monthly_stat.utcDate_.toString("yyyy-MM");
            monthly_stat.cumulateRetainedEarning();
            monthly_stats_.push_back(monthly_stat);

            month = month.addMonths(1);
            monthly_stat.clear(Account::Revenue);
            monthly_stat.clear(Account::Expense);
        }

        monthly_stat.cumulateCurrencyError(transaction.date_time.toUTC().date());  // This is the most time consuming part because this loop through all accounts in monthly_stat, which is ALL accounts in db.
        monthly_stat.cumulateTransaction(transaction);
    }
    // Push the last month summary which might be incomplete.
    monthly_stat.description = monthly_stat.utcDate_.toString("yyyy-MM-dd");  // The last "incomplete" month will have date as a distinguisher.
    monthly_stat.cumulateRetainedEarning();
    monthly_stats_.push_back(monthly_stat);

    refreshTableWidget();
}

QPair<QDate, FinancialStat> FinancialStatement::getStartStateFor(QDate query_date) {
    if (!monthly_stats_.empty() && monthly_stats_.back().description.length() == 10) {
        // Description in format "yyyy-MM-dd", which indicate this is a incomplete monthly summary.
        monthly_stats_.pop_back();
    }

    while (!monthly_stats_.empty()) {
        QDate last_record_date = monthly_stats_.back().utcDate_;
        QDate start_month = QDate(last_record_date.year(), last_record_date.month(), 1).addMonths(1).addDays(-1); // Last day of this month.
        if (start_month < query_date) {
            FinancialStat stat = monthly_stats_.back();
            stat.clear(Account::Revenue);
            stat.clear(Account::Expense);
            refreshTableWidget();
            return {start_month.addDays(1), stat};
        }
        monthly_stats_.pop_back();
    }

    QDate date = book_.getFirstTransactionDateTime().toUTC().date();
    refreshTableWidget();
    return {QDate(date.year(), date.month(), 1), FinancialStat()};
}

void FinancialStatement::on_comboBoxHousehold_currentIndexChanged(int /* index */) {
    refreshTableWidget();
}

