#include "home_window.h"
#include "ui_home_window.h"

#include <QMessageBox>
#include <QLineEdit>

#include "book/transaction.h"
#include "add_transaction/add_transaction.h"
#include "currency/currency.h"
#include "investment_analysis/investment_analysis.h"
#include "financial_statement/financial_statement.h"

HomeWindow::HomeWindow(QWidget *parent)
    : QMainWindow(parent),
      book("Book.db"),
      user_id(book.getLastLoggedInUserId()),
      ui(new Ui::HomeWindow),
      transactions_model_(this) {

    ui->setupUi(this);
    g_currency.openDatabase();
    account_manager_   = QSharedPointer<AccountManager>  (new AccountManager(this));
    household_manager_ = QSharedPointer<HouseholdManager>(new HouseholdManager(this));
    ui->tableView->setModel(&transactions_model_);
    ui->tableView->hideColumn(6);  // hide transaction_id

    // Init the filter elements.
    // Init start date.
    ui->dateEditFrom->setDate(QDate::currentDate().addMonths(-1));
    ui->dateEditFrom->setDisplayFormat("yyyy-MM-dd");
    connect(ui->dateEditFrom, &QDateEdit::userDateChanged, this, &HomeWindow::refreshTable);
    // Init end date.
    ui->dateEditTo->setDate(QDate(QDate::currentDate()));
    ui->dateEditTo->setDisplayFormat("yyyy-MM-dd");
    connect(ui->dateEditTo, &QDateEdit::userDateChanged, this, &HomeWindow::refreshTable);

    // Init combo box filters.
    name_combo_boxes_     = {ui->comboBoxExpenseName,     ui->comboBoxRevenueName,     ui->comboBoxAssetName,     ui->comboBoxLiabilityName};
    category_combo_boxes_ = {ui->comboBoxExpenseCategory, ui->comboBoxRevenueCategory, ui->comboBoxAssetCategory, ui->comboBoxLiabilityCategory};
    for (int i = 0; i < kAccountTypes.size(); i++) {
        // Refresh display when <name_combo_box> changed.
        connect(name_combo_boxes_[i], QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HomeWindow::refreshTable);

        auto table_type = kAccountTypes.at(i);
        // Update <name_combo_box> when <category_combo_box> changed.
        connect(category_combo_boxes_[i], &QComboBox::currentTextChanged, this, [=](const QString& new_category_name) { accountCategoryChanged(table_type, new_category_name, name_combo_boxes_[i]); });
    }
    setCategoryComboBox();
    // Put this to the last of init because this will triger on_tableView_transactions_cellChanged().
    // QLineEdit: Description Filter.
    ui->lineEditDescriptionFilter->setPlaceholderText("Description Filter");
    connect(ui->lineEditDescriptionFilter, &QLineEdit::textEdited, this, &HomeWindow::refreshTable);

    connect(ui->actionAccountManager,        &QAction::triggered, this, &HomeWindow::onActionAccountManagerTriggered);
    connect(ui->actionHouseholdManager,      &QAction::triggered, this, &HomeWindow::onActionHouseholdManagerTriggered);
    connect(ui->actionAddTransaction,        &QAction::triggered, this, &HomeWindow::onActionAddTransactionTriggered);
    connect(ui->actionTransactionValidation, &QAction::triggered, this, &HomeWindow::onActionTransactionValidationTriggered);
    connect(ui->actionFinancialStatement,    &QAction::triggered, this, &HomeWindow::onActionFinancialStatementTriggered);
    connect(ui->actionInvestmentAnalysis,    &QAction::triggered, this, &HomeWindow::onActionInvestmentAnalysisTriggered);
    connect(ui->actionLogin,                 &QAction::triggered, this, &HomeWindow::onActionLoginTriggered);
    connect(ui->actionLogout,                &QAction::triggered, this, &HomeWindow::onActionLogoutTriggered);

    connect(ui->pushButtonDelete, &QPushButton::clicked, this, &HomeWindow::onPushButtonDeleteClicked);
    connect(ui->pushButtonMerge,  &QPushButton::clicked, this, &HomeWindow::onPushButtonMergeClicked);
    connect(ui->tableView, &QTableView::doubleClicked, this, &HomeWindow::onTableViewDoubleClicked);

    refreshTable();
}

HomeWindow::~HomeWindow() {
    delete ui;
}

void HomeWindow::resizeEvent(QResizeEvent* event) {
    resizeTableView(ui->tableView);
    resizeTableView(ui->tableView);
    QMainWindow::resizeEvent(event);
}

void HomeWindow::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);
    // Do something on close here
}

void HomeWindow::refreshTable() {
    // Build transaction filter.
    TransactionFilter filter = TransactionFilter()
                               .fromTime(QDateTime(ui->dateEditFrom->date(), QTime(00, 00, 00)))
                                   .toTime(QDateTime(ui->dateEditTo->date(), QTime(23, 59, 59)))
                                   .setDescription(ui->lineEditDescriptionFilter->text())
                               .useAnd()
                               .orderByDescending()
                               .setLimit(200);
    for (int i = 0; i < kAccountTypes.size(); i++) {
        if (!name_combo_boxes_.at(i)->currentText().isEmpty()) {
            filter.addAccount(name_combo_boxes_.at(i)->currentData().value<QSharedPointer<Account>>());
        }
    }
    transactions_model_.setFilter(filter);
    resizeTableView(ui->tableView);
}

void HomeWindow::onTableViewDoubleClicked(const QModelIndex &index) {
    int row = index.row();
    Transaction transaction = transactions_model_.getTransaction(row);
    AddTransaction *add_transaction = new AddTransaction(this);
    add_transaction->setAttribute(Qt::WA_DeleteOnClose);
    add_transaction->setTransaction(transaction);
}

void HomeWindow::setCategoryComboBox() {
    for (int i = 0; i < kAccountTypes.size(); i++) {
        QComboBox *cateComboBox = category_combo_boxes_.at(i);
        cateComboBox->clear();
        cateComboBox->addItem("");
        cateComboBox->addItems(book.queryCategories(user_id, kAccountTypes.at(i)));
    }
}

void HomeWindow::accountCategoryChanged(const Account::Type& table_type,
                                        const QString& new_category_name,
                                        QComboBox* name_combo_box) {
    name_combo_box->clear();
    for (QSharedPointer<Account>& account_ptr : book.queryAccountNamesByLastUpdate(user_id, table_type, new_category_name, ui->dateEditTo->dateTime())) {
        name_combo_box->addItem(account_ptr->accountName(), QVariant::fromValue(account_ptr));
    }
}

void HomeWindow::resizeTableView(QTableView* table_view) {
    int width = table_view->width() - /*Tried it out, otherwise scroll bar still show up*/50;
    table_view->resizeColumnsToContents();
    width -= table_view->columnWidth(0); // Date & Time

    QSet<int> columns = {1, 2, 3, 4, 5};  // Columns to be adjusted.
    // Work on column 1.
    if (!ui->lineEditDescriptionFilter->text().isEmpty()) {
        width -= table_view->columnWidth(1);
        columns.remove(1);
    }
    // Work on column 2 to 5.
    for (int i = 0; i < name_combo_boxes_.size(); ++i) {
        if (!name_combo_boxes_.at(i)->currentText().isEmpty()) {
            width -= table_view->columnWidth(i + 2);
            columns.remove(i + 2);
        }
    }
    // Remove columns already narrower than limit.
    while (true) {
        int original_size = columns.size();
        for (int col : columns) {
            if (table_view->columnWidth(col) < width / columns.size()) {
                width -= table_view->columnWidth(col);
                columns.remove(col);
            }
        }
        if (original_size == columns.size()) {
            break;
        }
    }
    // Final adjustment.
    for (int col : columns) {
        table_view->setColumnWidth(col, width / columns.size());
    }
    table_view->resizeRowsToContents();
}

void HomeWindow::onPushButtonMergeClicked() {
    QSet<int> rows;
    for (QModelIndex index : ui->tableView->selectionModel()->selectedIndexes()) {
        if (index.isValid()) {
            rows.insert(index.row());
        }
    }
    if (rows.size() <= 1) {
        QMessageBox::warning(this, "Warning", "Need to select more than one row to merge.", QMessageBox::Ok);
        return;
    }

    QMessageBox warningMsgBox;
    warningMsgBox.setText("Are you sure you want to merge the following transactions?");
    for (int row : rows) {
        warningMsgBox.setInformativeText(warningMsgBox.informativeText()
                                         + '\n' + transactions_model_.data(row, 0).toDateTime().toString(kDateTimeFormat)
                                         + ": " + transactions_model_.data(row, 1).toString());
    }
    warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    warningMsgBox.setDefaultButton(QMessageBox::Cancel);
    switch (warningMsgBox.exec()) {
    case QMessageBox::Ok: {
        Transaction merged_transaction;
        for (int row : rows) {
            Transaction transaction = transactions_model_.getTransaction(row);
            merged_transaction += transaction;
            book.removeTransaction(transaction.id);
        }
        book.insertTransaction(user_id, merged_transaction);
        refreshTable();
        break;
    }
    case QMessageBox::Cancel:
        return;
    }
}

void HomeWindow::onPushButtonDeleteClicked() {
    QSet<int> rows;
    for (QModelIndex index : ui->tableView->selectionModel()->selectedIndexes()) {
        if (index.isValid()) {
            rows.insert(index.row());
        }
    }
    if (rows.empty()) {
        QMessageBox::warning(this, "Warning", "No transaction was selected.", QMessageBox::Ok);
        return;
    }

    QMessageBox warningMsgBox;
    warningMsgBox.setText("Are you sure you want to delete the following transactions?");
    for (int row : rows) {
        warningMsgBox.setInformativeText(warningMsgBox.informativeText()
                                         + '\n' + transactions_model_.data(row, 0).toDateTime().toString(kDateTimeFormat)
                                         + ": " + transactions_model_.data(row, 1).toString());
    }
    warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    warningMsgBox.setDefaultButton(QMessageBox::Cancel);

    switch (warningMsgBox.exec()) {
    case QMessageBox::Ok:
        for (int row : rows) {
            book.removeTransaction(transactions_model_.data(row, 6).toUInt());
        }
        refreshTable();
        break;
    case QMessageBox::Cancel:
        return;
    }
}

void HomeWindow::onActionAddTransactionTriggered() {
    AddTransaction* addTransaction = new AddTransaction(this);
    addTransaction->setAttribute(Qt::WA_DeleteOnClose);
    addTransaction->show();
}

void HomeWindow::onActionAccountManagerTriggered() {
    account_manager_->show();
}

void HomeWindow::onActionFinancialStatementTriggered() {
    FinancialStatement* financial_statement = new FinancialStatement(this);
    financial_statement->setAttribute(Qt::WA_DeleteOnClose);
    financial_statement->on_pushButton_Query_clicked();
    financial_statement->show();
}

void HomeWindow::onActionInvestmentAnalysisTriggered() {
    InvestmentAnalysis* investment_analysis = new InvestmentAnalysis(this);
    investment_analysis->setAttribute(Qt::WA_DeleteOnClose);
    investment_analysis->show();
}

void HomeWindow::onActionTransactionValidationTriggered() {
    // Display validation message
    QString errorMessage = "";
    // Query ALL transactions.
    for (const Transaction& transaction : book.queryTransactions(user_id)) {
        if (!transaction.validate().empty()) {
            errorMessage += transaction.date_time.toString("yyyy/MM/dd HH:mm:ss") + ": ";
            errorMessage += transaction.description + '\n';
            errorMessage += "\t" + transaction.validate().join("; ") + "\n\n";
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

void HomeWindow::onActionHouseholdManagerTriggered() {
    household_manager_->show();
}

void HomeWindow::onActionLoginTriggered() {
    // TODO: Change to username & password login.
    bool ok;
    int id = QInputDialog::getInt(this, tr("QInputDialog::getInteger()"), tr("User ID:"), 0, 1, 100, 1, &ok);
    if (ok) {
        user_id = id;
    }

    book.updateLoginTime(user_id);
    // TODO: Use signal / slot for user_id_ changed event.
    household_manager_->model_.setFilter(QString("user_id = %1").arg(user_id));
    setCategoryComboBox();
    refreshTable();
}

void HomeWindow::onActionLogoutTriggered() {
    user_id = -1;

    // TODO: Use signal / slot for user_id_ changed event.
    household_manager_->model_.setFilter(QString("user_id = %1").arg(user_id));
    refreshTable();
}
