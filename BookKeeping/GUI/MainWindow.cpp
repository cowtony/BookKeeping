#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>
#include <QLineEdit>

#include "Transaction.h"
#include "AddTransaction.h"
#include "Currency.h"
#include "InvestmentAnalysis.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow),
    TableIndex({{2, Account::Expense}, {3, Account::Revenue}, {4, Account::Asset}, {5, Account::Liability}})
{
    ui->setupUi(this);

    g_book.openDatabase("Book.db");
    g_currency.openDatabase();

    accountManager     = new AccountManager(this);
    financialStatement = new FinancialStatement(this);

    // Init table widget
    ui->tableWidget_transactions->setRowCount(2);

    startDate_ = new QDateEdit();
    startDate_->setDate(QDate::currentDate().addMonths(-1));
    startDate_->setDisplayFormat("yyyy-MM-dd");
    ui->tableWidget_transactions->setCellWidget(0, 0, startDate_);
    connect(startDate_, SIGNAL(userDateChanged(const QDate&)), this, SLOT(displayTransactions()));

    endDate_ = new QDateEdit();
    endDate_->setDate(QDate(QDate::currentDate()));
    endDate_->setDisplayFormat("yyyy-MM-dd");
    ui->tableWidget_transactions->setCellWidget(1, 0, endDate_);
    connect(endDate_, SIGNAL(userDateChanged(const QDate&)), this, SLOT(displayTransactions()));

    for (const Account::TableType &tableType : TableIndex.values())
    {
        QComboBox *cateComboBox = new QComboBox();
        ui->tableWidget_transactions->setCellWidget(0, TableIndex.key(tableType), cateComboBox);
        connect(cateComboBox, &QComboBox::currentTextChanged, [this, tableType](const QString& newCate){ accountCategoryChanged(tableType, newCate); });
//        connect(cateComboBox, &QComboBox::currentTextChanged, std::bind(&MainWindow::accountCategoryChanged, this, i, std::placeholders::_1));

        QComboBox *nameComboBox = new QComboBox();
        ui->tableWidget_transactions->setCellWidget(1, TableIndex.key(tableType), nameComboBox);
        connect(nameComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::displayTransactions);
    }
    setCategoryComboBox();

    // Put this to the last of init because this will triger on_tableWidget_transactions_cellChanged()
    ui->tableWidget_transactions->setItem(0, 1, new QTableWidgetItem("Description Filter:"));
    ui->tableWidget_transactions->item(0, 1)->setFlags(ui->tableWidget_transactions->item(0, 1)->flags() & ~Qt::ItemIsEnabled);
    QLineEdit *descriptionFilter = new QLineEdit;
    ui->tableWidget_transactions->setCellWidget(1, 1, descriptionFilter);
    connect(descriptionFilter, &QLineEdit::textEdited, this, &MainWindow::displayTransactions);

    connect(accountManager, &AccountManager::accountNameChanged, this, &MainWindow::displayTransactions);
    connect(accountManager, &AccountManager::categoryChanged, this, &MainWindow::setCategoryComboBox);
    connect(accountManager, &AccountManager::categoryChanged, this, &MainWindow::displayTransactions);
    displayTransactions();
}

MainWindow::~MainWindow()
{
    delete accountManager;
    delete financialStatement;

    delete ui;
}

void MainWindow::displayTransactions()
{
    QVector<QComboBox*> cateComboBoxs(4);
    QVector<QComboBox*> nameComboBoxs(4);
    for (int i = 0; i < 4; i++)
    {
        cateComboBoxs[i] = static_cast<QComboBox*>(ui->tableWidget_transactions->cellWidget(0, i + 2));
        nameComboBoxs[i] = static_cast<QComboBox*>(ui->tableWidget_transactions->cellWidget(1, i + 2));
        if (cateComboBoxs.at(i)->currentIndex() != 0 && nameComboBoxs.at(i)->currentIndex() == -1) return; // Can be deleted?
    }

    while (ui->tableWidget_transactions->rowCount() > 2)
        ui->tableWidget_transactions->removeRow(2);

    QList<Account> filter;
    if (!nameComboBoxs.at(0)->currentText().isEmpty())
        filter << Account(Account::Expense,   cateComboBoxs.at(0)->currentText(), nameComboBoxs.at(0)->currentText());
    if (!nameComboBoxs.at(1)->currentText().isEmpty())
        filter << Account(Account::Revenue,   cateComboBoxs.at(1)->currentText(), nameComboBoxs.at(1)->currentText());
    if (!nameComboBoxs.at(2)->currentText().isEmpty())
        filter << Account(Account::Asset,     cateComboBoxs.at(2)->currentText(), nameComboBoxs.at(2)->currentText());
    if (!nameComboBoxs.at(3)->currentText().isEmpty())
        filter << Account(Account::Liability, cateComboBoxs.at(3)->currentText(), nameComboBoxs.at(3)->currentText());

    MoneyArray expenseSum  (endDate_->date(), USD);
    MoneyArray revenueSum  (endDate_->date(), USD);
    MoneyArray assetSum    (endDate_->date(), USD);
    MoneyArray liabilitySum(endDate_->date(), USD);

    for (const Transaction& t : g_book.queryTransactions(QDateTime(startDate_->date(), QTime(00, 00, 00)),
                                                         QDateTime(endDate_->date(), QTime(23, 59, 59)),
                                                         static_cast<QLineEdit*>(ui->tableWidget_transactions->cellWidget(1, 1))->text(),
                                                         filter, false, false))
    {
        int row = ui->tableWidget_transactions->rowCount();
        if (row > 200) break;

        expenseSum   += t.getMoneyArray(Account(Account::Expense,   cateComboBoxs.at(0)->currentText(), nameComboBoxs.at(0)->currentText()));
        revenueSum   += t.getMoneyArray(Account(Account::Revenue,   cateComboBoxs.at(1)->currentText(), nameComboBoxs.at(1)->currentText()));
        assetSum     += t.getMoneyArray(Account(Account::Asset,     cateComboBoxs.at(2)->currentText(), nameComboBoxs.at(2)->currentText()));
        liabilitySum += t.getMoneyArray(Account(Account::Liability, cateComboBoxs.at(3)->currentText(), nameComboBoxs.at(3)->currentText()));

        ui->tableWidget_transactions->insertRow(row);
        ui->tableWidget_transactions->setItem(row, 0, new QTableWidgetItem(t.dateTime_.toString(DATE_TIME_FORMAT)));
        ui->tableWidget_transactions->setItem(row, 1, new QTableWidgetItem(t.description_));
        ui->tableWidget_transactions->setItem(row, 2, new QTableWidgetItem(t.dataToString(Account::Expense).replace("; ", "\n")));
        ui->tableWidget_transactions->setItem(row, 3, new QTableWidgetItem(t.dataToString(Account::Revenue).replace("; ", "\n")));
        ui->tableWidget_transactions->setItem(row, 4, new QTableWidgetItem(t.dataToString(Account::Asset).replace("; ", "\n")));
        ui->tableWidget_transactions->setItem(row, 5, new QTableWidgetItem(t.dataToString(Account::Liability).replace("; ", "\n")));
    }

    int row = ui->tableWidget_transactions->rowCount();
    ui->tableWidget_transactions->insertRow(row);
    ui->tableWidget_transactions->setItem(row, 1, new QTableWidgetItem("SUM:"));
    ui->tableWidget_transactions->setItem(row, 2, new QTableWidgetItem(expenseSum.toString()));
    ui->tableWidget_transactions->setItem(row, 3, new QTableWidgetItem(revenueSum.toString()));
    ui->tableWidget_transactions->setItem(row, 4, new QTableWidgetItem(assetSum.toString()));
    ui->tableWidget_transactions->setItem(row, 5, new QTableWidgetItem(liabilitySum.toString()));

    ui->tableWidget_transactions->resizeColumnsToContents();
    ui->tableWidget_transactions->resizeRowsToContents();
}

void MainWindow::on_tableWidget_transactions_cellDoubleClicked(int row, int column)
{
    if (row > 1)
    {
        Transaction t = g_book.getTransaction(QDateTime::fromString(ui->tableWidget_transactions->item(row, 0)->text(), DATE_TIME_FORMAT));
        AddTransaction *addT = new AddTransaction(this);
        addT->setAttribute(Qt::WA_DeleteOnClose);
        addT->setTransaction(t);
        connect(addT, &AddTransaction::insertTransactionFinished, this, &MainWindow::displayTransactions);
    }
}

void MainWindow::setCategoryComboBox()
{
    for (const int &col : TableIndex.keys())
    {
        QComboBox *cateComboBox = static_cast<QComboBox *>(ui->tableWidget_transactions->cellWidget(0, col));
        cateComboBox->clear();
        cateComboBox->addItem("");
        cateComboBox->addItems(g_book.getCategories(TableIndex.value(col)));
    }
}

void MainWindow::on_pushButton_MergeTransaction_clicked()
{
    QMap<QDateTime, QString> transactionInfos = getSelectedTransactionInfos();

    QMessageBox warningMsgBox;
    warningMsgBox.setText("Are you sure you want to merge the following transactions?");
    for (const QDateTime &dateTime : transactionInfos.keys()) {
        warningMsgBox.setInformativeText(warningMsgBox.informativeText()
                                         + '\n' + dateTime.toString(DATE_TIME_FORMAT)
                                         + ": " + transactionInfos.value(dateTime));
    }
    warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    warningMsgBox.setDefaultButton(QMessageBox::Cancel);
    switch (warningMsgBox.exec()) {
    case QMessageBox::Ok:
    {
        Transaction t;
        for (const QDateTime &dateTime : transactionInfos.keys()) {
            t += g_book.getTransaction(dateTime);
            g_book.removeTransaction(dateTime);
        }
        g_book.insertTransaction(t);
        displayTransactions();
        break;
    }
    case QMessageBox::Cancel:
        return;
    }
}

void MainWindow::on_pushButtonDeleteTransactions_clicked()
{
    QMap<QDateTime, QString> transactionInfos = getSelectedTransactionInfos();
    if (transactionInfos.empty()) return;

    QMessageBox warningMsgBox;
    warningMsgBox.setText("Are you sure you want to delete the following transactions?");
    for (const QDateTime &dateTime : transactionInfos.keys())
        warningMsgBox.setInformativeText(warningMsgBox.informativeText()
                                         + '\n' + dateTime.toString(DATE_TIME_FORMAT)
                                         + ": " + transactionInfos.value(dateTime));
    warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    warningMsgBox.setDefaultButton(QMessageBox::Cancel);
    switch (warningMsgBox.exec()) {
    case QMessageBox::Ok:
        for (const QDateTime &dateTime : transactionInfos.keys())
            g_book.removeTransaction(dateTime);
        displayTransactions();
        break;
    case QMessageBox::Cancel:
        return;
    }
}

QMap<QDateTime, QString> MainWindow::getSelectedTransactionInfos() const
{
    QList<QTableWidgetSelectionRange> ranges = ui->tableWidget_transactions->selectedRanges();
    QMap<QDateTime, QString> transactionInfos;
    for (const QTableWidgetSelectionRange &range: ranges)
    {
        for (int row = range.topRow(); row <= range.bottomRow(); row++)
        {
            if (row < 2) continue; // Skip header rows
            transactionInfos.insert(QDateTime::fromString(ui->tableWidget_transactions->item(row, 0)->text(), DATE_TIME_FORMAT),
                                                          ui->tableWidget_transactions->item(row, 1)->text());
        }
    }
    return transactionInfos;
}

void MainWindow::on_actionAddTransaction_triggered() {
  AddTransaction* addTransaction = new AddTransaction(this);
  addTransaction->setAttribute(Qt::WA_DeleteOnClose);
  connect(addTransaction, &AddTransaction::insertTransactionFinished, this, &MainWindow::displayTransactions);
  addTransaction->show();
}

void MainWindow::accountCategoryChanged(const Account::TableType &tableType, const QString &category) {
  QComboBox* nameComboBox = static_cast<QComboBox*>(ui->tableWidget_transactions->cellWidget(1, TableIndex.key(tableType)));
  nameComboBox->clear();
  nameComboBox->addItems(g_book.getAccountNamesByLastUpdate(tableType, category, endDate_->dateTime()));
}

void MainWindow::on_actionAccountManager_triggered() {
  accountManager->show();
}

void MainWindow::on_actionFinancialStatement_triggered() {
  // TODO: can I declare obj here instead of declare in class?
  financialStatement->on_pushButton_Query_clicked();
  financialStatement->show();
}

void MainWindow::on_actionInvestmentAnalysis_triggered() {
  InvestmentAnalysis* investmentAnalysis = new InvestmentAnalysis(this);
  investmentAnalysis->show();
}

void MainWindow::on_actionTransactionValidation_triggered() {
  // Display validation message
  QString errorMessage = "";
  for (const Transaction& t : g_book.queryTransactions(QDateTime(QDate(1990, 05, 25), QTime(0, 0, 0)),
                                                       QDateTime(QDate(2200, 1, 1), QTime(0, 0, 0)), "", {}, false)) {
    if (!t.validation().empty()) {
      errorMessage += t.dateTime_.toString("yyyy/MM/dd HH:mm:ss") + ": ";
      errorMessage += t.description_ + '\n';
      errorMessage += "\t" + t.validation().join("; ") + "\n\n";
    }
  }

  QMessageBox msgBox;
  if (!errorMessage.isEmpty()) {
    msgBox.setText("The following transaction(s) not passing validation:");
  } else {
    msgBox.setText("No invalid transaction!");
  }
  msgBox.setInformativeText(errorMessage);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  QMainWindow::closeEvent(event);
  // Do something on close here
}
