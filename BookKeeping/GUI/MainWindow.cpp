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

  ui->tableView_transactions->setModel(&book_model_);

  // Init the filter elements.
  // Init start date.
  start_date_ = new QDateEdit();
  start_date_->setDate(QDate::currentDate().addMonths(-1));
  start_date_->setDisplayFormat("yyyy-MM-dd");
  ui->tableView_transactions->setIndexWidget(book_model_.index(0, 0), start_date_);
  connect(start_date_, SIGNAL(userDateChanged(const QDate&)), this, SLOT(refreshTable()));
  // Init end date.
  end_date_ = new QDateEdit();
  end_date_->setDate(QDate(QDate::currentDate()));
  end_date_->setDisplayFormat("yyyy-MM-dd");
  ui->tableView_transactions->setIndexWidget(book_model_.index(1, 0), end_date_);
  connect(end_date_, SIGNAL(userDateChanged(const QDate&)), this, SLOT(refreshTable()));
  // Init combo box filters.
  for (int i = 0; i < kTableList.size(); i++) {
    QComboBox *name_combo_box = new QComboBox();
    name_combo_boxes_.push_back(name_combo_box);
    ui->tableView_transactions->setIndexWidget(book_model_.index(1, i + 2), name_combo_box);
    // Refresh display when <name_combo_box> changed.
    connect(name_combo_box, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::refreshTable);

    QComboBox *cate_combo_box = new QComboBox();
    category_combo_boxes_.push_back(cate_combo_box);
    ui->tableView_transactions->setIndexWidget(book_model_.index(0, i + 2), cate_combo_box);
    auto table_type = kTableList.at(i);
    // Update <name_combo_box> when <category_combo_box> changed.
    connect(cate_combo_box, &QComboBox::currentTextChanged, [=](const QString& new_category_name){ accountCategoryChanged(table_type, new_category_name, name_combo_box); });
  }
  setCategoryComboBox();
  // Put this to the last of init because this will triger on_tableView_transactions_cellChanged().
  // QLineEdit: Description Filter.
  QLineEdit *description_filter = new QLineEdit;
  description_filter->setPlaceholderText("Description Filter");
  ui->tableView_transactions->setIndexWidget(book_model_.index(1, 1), description_filter);
  connect(description_filter, &QLineEdit::textEdited, this, &MainWindow::refreshTable);

  connect(account_manager_, &AccountManager::accountNameChanged, this, &MainWindow::refreshTable);
  connect(account_manager_, &AccountManager::categoryChanged, this, &MainWindow::setCategoryComboBox);
  connect(account_manager_, &AccountManager::categoryChanged, this, &MainWindow::refreshTable);

  refreshTable();
}

MainWindow::~MainWindow() {
  delete account_manager_;
  delete financial_statement_;

  delete ui;
}

void MainWindow::refreshTable() {
  // Build transaction filter.
  TransactionFilter filter(QDateTime(start_date_->date(), QTime(00, 00, 00)),
                           QDateTime(end_date_->date(), QTime(23, 59, 59)),
                           static_cast<QLineEdit*>(ui->tableView_transactions->indexWidget(book_model_.index(1, 1)))->text(),
                           {}, false, false);
  for (int i = 0; i < kTableList.size(); i++) {
    if (!name_combo_boxes_.at(i)->currentText().isEmpty()) {
      filter.addAccount(Account(kTableList.at(i), category_combo_boxes_.at(i)->currentText(), name_combo_boxes_.at(i)->currentText()));
    }
  }

  book_model_.SetTransactions(book_.queryTransactions(filter));

  // Adjust column width & row height.
  ui->tableView_transactions->resizeColumnsToContents();
  ui->tableView_transactions->resizeRowsToContents();
}

void MainWindow::on_tableView_transactions_doubleClicked(const QModelIndex &index) {
  if (index.row() >= kReservedFilterRow) {
    Transaction transaction = book_model_.getTransaction(index.row() - kReservedFilterRow);
    AddTransaction *add_transaction = new AddTransaction(book_, this);
    add_transaction->setAttribute(Qt::WA_DeleteOnClose);
    add_transaction->setTransaction(transaction);
    connect(add_transaction, &AddTransaction::insertTransactionFinished, this, &MainWindow::refreshTable);
  }
}

void MainWindow::setCategoryComboBox() {
  for (int i = 0; i < kTableList.size(); i++) {
    QComboBox *cateComboBox = category_combo_boxes_.at(i);
    cateComboBox->clear();
    cateComboBox->addItem("");
    cateComboBox->addItems(book_.getCategories(kTableList.at(i)));
  }
}

void MainWindow::on_pushButton_MergeTransaction_clicked() {
  QList<Transaction> selected_transactions = book_model_.getTransactions(ui->tableView_transactions->selectionModel()->selectedIndexes());
  if (selected_transactions.size() <= 1) {
    QMessageBox::warning(this, "Warning", "No transaction or only one transaction were selected.", QMessageBox::Ok);
    return;
  }
  QMessageBox warningMsgBox;
  warningMsgBox.setText("Are you sure you want to merge the following transactions?");
  for (const Transaction& transaction : selected_transactions) {
    warningMsgBox.setInformativeText(warningMsgBox.informativeText()
                                     + '\n' + transaction.date_time_.toString(DATE_TIME_FORMAT)
                                     + ": " + transaction.description_);
  }
  warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  warningMsgBox.setDefaultButton(QMessageBox::Cancel);
  switch (warningMsgBox.exec()) {
  case QMessageBox::Ok: {
    Transaction merged_transaction;
    for (const Transaction& transaction : selected_transactions) {
      merged_transaction += transaction;
      book_.removeTransaction(transaction.date_time_);
    }
    book_.insertTransaction(merged_transaction);
    refreshTable();
    break;
  }
  case QMessageBox::Cancel:
    return;
  }
}

void MainWindow::on_pushButtonDeleteTransactions_clicked() {
  QList<Transaction> selected_transactions = book_model_.getTransactions(ui->tableView_transactions->selectionModel()->selectedIndexes());
  if (selected_transactions.empty()) {
    QMessageBox::warning(this, "Warning", "No transaction was selected.", QMessageBox::Ok);
    return;
  }
  QMessageBox warningMsgBox;
  warningMsgBox.setText("Are you sure you want to delete the following transactions?");
  for (const Transaction& transaction : selected_transactions)
      warningMsgBox.setInformativeText(warningMsgBox.informativeText()
                                       + '\n' + transaction.date_time_.toString(DATE_TIME_FORMAT)
                                       + ": " + transaction.description_);
  warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  warningMsgBox.setDefaultButton(QMessageBox::Cancel);
  switch (warningMsgBox.exec()) {
  case QMessageBox::Ok:
    for (const Transaction& transaction : selected_transactions) {
      book_.removeTransaction(transaction.date_time_);
    }
    refreshTable();
    break;
  case QMessageBox::Cancel:
    return;
  }
}

void MainWindow::on_actionAddTransaction_triggered() {
  AddTransaction* addTransaction = new AddTransaction(book_, this);
  addTransaction->setAttribute(Qt::WA_DeleteOnClose);
  connect(addTransaction, &AddTransaction::insertTransactionFinished, this, &MainWindow::refreshTable);
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
  // Query ALL transactions.
  for (const Transaction& transaction : book_.queryTransactions(TransactionFilter())) {
    if (!transaction.validation().empty()) {
      errorMessage += transaction.date_time_.toString("yyyy/MM/dd HH:mm:ss") + ": ";
      errorMessage += transaction.description_ + '\n';
      errorMessage += "\t" + transaction.validation().join("; ") + "\n\n";
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
