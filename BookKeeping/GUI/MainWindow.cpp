#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>
#include <QLineEdit>

#include "Transaction.h"
#include "AddTransaction.h"
#include "Currency.h"
#include "InvestmentAnalysis.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    ui(new Ui::MainWindow),
    book_("Book.db") {
  ui->setupUi(this);

  g_currency.openDatabase();
  account_manager_     = new AccountManager(book_, this);
  financial_statement_ = new FinancialStatement(book_, this);

  // Init table widget.
  ui->tableWidget_transactions->setRowCount(2);
  // Init start date.
  start_date_ = new QDateEdit();
  start_date_->setDate(QDate::currentDate().addMonths(-1));
  start_date_->setDisplayFormat("yyyy-MM-dd");
  ui->tableWidget_transactions->setCellWidget(0, 0, start_date_);
  connect(start_date_, SIGNAL(userDateChanged(const QDate&)), this, SLOT(displayTransactions()));
  // Init end date.
  end_date_ = new QDateEdit();
  end_date_->setDate(QDate(QDate::currentDate()));
  end_date_->setDisplayFormat("yyyy-MM-dd");
  ui->tableWidget_transactions->setCellWidget(1, 0, end_date_);
  connect(end_date_, SIGNAL(userDateChanged(const QDate&)), this, SLOT(displayTransactions()));
  // Init combo box filters.
  for (int i = 0; i < kTableList.size(); i++) {
    QComboBox *name_combo_box = new QComboBox();
    name_combo_boxes_.push_back(name_combo_box);
    ui->tableWidget_transactions->setCellWidget(1, i + 2, name_combo_box);
    // Refresh display when <name_combo_box> changed.
    connect(name_combo_box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::displayTransactions);

    QComboBox *cate_combo_box = new QComboBox();
    category_combo_boxes_.push_back(cate_combo_box);
    ui->tableWidget_transactions->setCellWidget(0, i + 2, cate_combo_box);
    auto table_type = kTableList.at(i);
    // Update <name_combo_box> when <category_combo_box> changed.
    connect(cate_combo_box, &QComboBox::currentTextChanged, [=](const QString& new_category_name){ accountCategoryChanged(table_type, new_category_name, name_combo_box); });
  }
  setCategoryComboBox();

  // Put this to the last of init because this will triger on_tableWidget_transactions_cellChanged()
  ui->tableWidget_transactions->setItem(0, 1, new QTableWidgetItem("Description Filter:"));
  ui->tableWidget_transactions->item(0, 1)->setFlags(ui->tableWidget_transactions->item(0, 1)->flags() & ~Qt::ItemIsEnabled);
  QLineEdit *descriptionFilter = new QLineEdit;
  ui->tableWidget_transactions->setCellWidget(1, 1, descriptionFilter);
  connect(descriptionFilter, &QLineEdit::textEdited, this, &MainWindow::displayTransactions);

  connect(account_manager_, &AccountManager::accountNameChanged, this, &MainWindow::displayTransactions);
  connect(account_manager_, &AccountManager::categoryChanged, this, &MainWindow::setCategoryComboBox);
  connect(account_manager_, &AccountManager::categoryChanged, this, &MainWindow::displayTransactions);
  displayTransactions();
}

MainWindow::~MainWindow() {
  delete account_manager_;
  delete financial_statement_;

  delete ui;
}

void MainWindow::displayTransactions() {
  const int kMaximumRows = 200;
  while (ui->tableWidget_transactions->rowCount() > 2)
      ui->tableWidget_transactions->removeRow(2);

  // Collecxt filters.
  QList<Account> filter;
  for (int i = 0; i < kTableList.size(); i++) {
    if (!name_combo_boxes_.at(i)->currentText().isEmpty()) {
      filter << Account(kTableList.at(i), category_combo_boxes_.at(i)->currentText(), name_combo_boxes_.at(i)->currentText());
    }
  }

  // Calculate sum and display to window.
  QVector<MoneyArray> sums(kTableList.size(), MoneyArray{end_date_->date(), USD});
  for (const Transaction& t : book_.queryTransactions(QDateTime(start_date_->date(), QTime(00, 00, 00)),
                                                      QDateTime(end_date_->date(), QTime(23, 59, 59)),
                                                      static_cast<QLineEdit*>(ui->tableWidget_transactions->cellWidget(1, 1))->text(),
                                                      filter, false, false)) {
    int row = ui->tableWidget_transactions->rowCount();
    if (row > kMaximumRows) {
      break;
    }

    for (int i = 0; i < kTableList.size(); i++) {
      sums[i] += t.getMoneyArray(Account(kTableList.at(i),
                                         category_combo_boxes_.at(i)->currentText(),
                                         name_combo_boxes_.at(i)->currentText()));
    }

    ui->tableWidget_transactions->insertRow(row);
    ui->tableWidget_transactions->setItem(row, 0, new QTableWidgetItem(t.date_time_.toString(DATE_TIME_FORMAT)));
    ui->tableWidget_transactions->setItem(row, 1, new QTableWidgetItem(t.description_));
    for (int i = 0; i < kTableList.size(); i++) {
      ui->tableWidget_transactions->setItem(row, i +  2, new QTableWidgetItem(t.dataToString(kTableList.at(i)).replace("; ", "\n")));
    }
  }

  // Add last row for sum.
  int row = ui->tableWidget_transactions->rowCount();
  ui->tableWidget_transactions->insertRow(row);
  ui->tableWidget_transactions->setItem(row, 1, new QTableWidgetItem("SUM:"));
  for (int i = 0; i < kTableList.size(); i++) {
    ui->tableWidget_transactions->setItem(row, i + 2, new QTableWidgetItem(sums.at(i).toString()));
  }

  // Adjust column width & row height.
  ui->tableWidget_transactions->resizeColumnsToContents();
  ui->tableWidget_transactions->resizeRowsToContents();
}

void MainWindow::on_tableWidget_transactions_cellDoubleClicked(int row, int column) {
  if (row > 1) {
    Transaction t = book_.getTransaction(QDateTime::fromString(ui->tableWidget_transactions->item(row, 0)->text(), DATE_TIME_FORMAT));
    AddTransaction *addT = new AddTransaction(book_, this);
    addT->setAttribute(Qt::WA_DeleteOnClose);
    addT->setTransaction(t);
    connect(addT, &AddTransaction::insertTransactionFinished, this, &MainWindow::displayTransactions);
  }
}

void MainWindow::setCategoryComboBox() {
  for (int i = 0; i < kTableList.size(); i++) {
    QComboBox *cateComboBox = static_cast<QComboBox *>(ui->tableWidget_transactions->cellWidget(0, i + 2));
    cateComboBox->clear();
    cateComboBox->addItem("");
    cateComboBox->addItems(book_.getCategories(kTableList.at(i)));
  }
}

void MainWindow::on_pushButton_MergeTransaction_clicked() {
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
  case QMessageBox::Ok: {
    Transaction t;
    for (const QDateTime &dateTime : transactionInfos.keys()) {
      t += book_.getTransaction(dateTime);
      book_.removeTransaction(dateTime);
    }
    book_.insertTransaction(t);
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
            book_.removeTransaction(dateTime);
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
  AddTransaction* addTransaction = new AddTransaction(book_, this);
  addTransaction->setAttribute(Qt::WA_DeleteOnClose);
  connect(addTransaction, &AddTransaction::insertTransactionFinished, this, &MainWindow::displayTransactions);
  addTransaction->show();
}

void MainWindow::accountCategoryChanged(const Account::TableType& table_type,
                                        const QString& category,
                                        QComboBox* name_combo_box) {
  name_combo_box->clear();
  name_combo_box->addItems(book_.getAccountNamesByLastUpdate(table_type, category, end_date_->dateTime()));
}

void MainWindow::on_actionAccountManager_triggered() {
  account_manager_->show();
}

void MainWindow::on_actionFinancialStatement_triggered() {
  // TODO: can I declare obj here instead of declare in class?
  financial_statement_->on_pushButton_Query_clicked();
  financial_statement_->show();
}

void MainWindow::on_actionInvestmentAnalysis_triggered() {
  InvestmentAnalysis* investmentAnalysis = new InvestmentAnalysis(book_, this);
  investmentAnalysis->show();
}

void MainWindow::on_actionTransactionValidation_triggered() {
  // Display validation message
  QString errorMessage = "";
  for (const Transaction& t : book_.queryTransactions(QDateTime(QDate(1990, 05, 25), QTime(0, 0, 0)),
                                                       QDateTime(QDate(2200, 1, 1), QTime(0, 0, 0)), "", {}, false)) {
    if (!t.validation().empty()) {
      errorMessage += t.date_time_.toString("yyyy/MM/dd HH:mm:ss") + ": ";
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
