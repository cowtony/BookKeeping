#include "home_window.h"
#include "ui_home_window.h"

#include <QMessageBox>
#include <QLineEdit>

#include "book/transaction.h"
#include "add_transaction/add_transaction.h"
#include "currency/currency.h"
#include "investment_analysis/investment_analysis.h"

HomeWindow::HomeWindow(QWidget *parent)
  : QMainWindow(parent),
    book("Book.db"),
    user_id(book.getLastLoggedInUserId()),
    household_manager(this), financial_statement(this),
    ui(new Ui::HomeWindow),
    transactions_model_(this) {

    ui->setupUi(this);
    g_currency.openDatabase();

    ui->tableView->setModel(&transactions_model_);
    ui->tableView->hideColumn(TransactionsModel::kTransactionIdColumnIndex);  // hide transaction_id
    ui->tableView->hideColumn(TransactionsModel::kTimeZoneColumnIndex);  // hide time_zone

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
        connect(category_combo_boxes_[i], &QComboBox::currentIndexChanged, this, [this, i, table_type](int /*index*/) {
            accountCategoryChanged(table_type, category_combo_boxes_[i], name_combo_boxes_[i]);
        });
    }
    initCategoryComboBox();
    // Put this to the last of init because this will triger on_tableView_transactions_cellChanged().
    // QLineEdit: Description Filter.
    ui->lineEditDescriptionFilter->setPlaceholderText("Description Filter");
    connect(ui->lineEditDescriptionFilter, &QLineEdit::textEdited, this, &HomeWindow::refreshTable);

    ui->comboBoxTimeZone->addItem("");
    AddTransaction::setupTimeZoneComboBox(ui->comboBoxTimeZone);
    ui->comboBoxTimeZone->setCurrentText("");
    connect(ui->comboBoxTimeZone, &QComboBox::currentIndexChanged, this, &HomeWindow::refreshTable);

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
                                   .startTime(QDateTime(ui->dateEditFrom->date(), QTime(00, 00, 00)))
                                   .endTime(QDateTime(ui->dateEditTo->date(), QTime(23, 59, 59)))
                                   .setDescription(ui->lineEditDescriptionFilter->text())
                               .useAnd()
                               .orderByDescending()
                               .setLimit(200);

    filter.timeZone = ui->comboBoxTimeZone->currentText();

    for (int i = 0; i < kAccountTypes.size(); i++) {
        if (name_combo_boxes_.at(i)->count() == 0) {
            continue;  // Skip because when `name_combo_box.clear()`, this function will also be triggered.
        }
        if (!name_combo_boxes_.at(i)->currentText().isEmpty()) {
            auto account = name_combo_boxes_.at(i)->currentData().value<QSharedPointer<Account>>();
            filter.addAccount(account);
        } else if (!category_combo_boxes_.at(i)->currentText().isEmpty()) {
            auto category = category_combo_boxes_.at(i)->currentData().value<QSharedPointer<Account>>();
            filter.addAccount(category);
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

void HomeWindow::initCategoryComboBox() {
    for (int i = 0; i < kAccountTypes.size(); i++) {
        QComboBox *cateComboBox = category_combo_boxes_.at(i);
        cateComboBox->blockSignals(true);
        cateComboBox->clear();
        cateComboBox->addItem("", QVariant::fromValue(Account::create(-1, -1, kAccountTypes.at(i), "", "")));
        for (QSharedPointer<Account>& category : book.getCategories(user_id, kAccountTypes.at(i))) {
            cateComboBox->addItem(category->categoryName(), QVariant::fromValue(category));
        }
        cateComboBox->blockSignals(false);
    }
}

void HomeWindow::accountCategoryChanged(const Account::Type& table_type, QComboBox* category_combo_box, QComboBox* name_combo_box) {
    name_combo_box->clear();
    QSharedPointer<Account> category = category_combo_box->currentData().value<QSharedPointer<Account>>();
    // Add a blank account, this can filter all accounts for this category.
    name_combo_box->addItem("", QVariant::fromValue(Account::create(-1, category->categoryId(), table_type, category->categoryName(), "")));
    for (QSharedPointer<Account>& account : book.queryAccountNamesByLastUpdate(user_id,
                                                                               table_type,
                                                                               category->categoryName(),
                                                                               ui->dateEditTo->dateTime())) {
        name_combo_box->addItem(account->accountName(), QVariant::fromValue(account));
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
    QList<Transaction> transactions;
    QStringList transactionSummary;
    for (QModelIndex index : ui->tableView->selectionModel()->selectedIndexes()) {
        if (index.isValid()) {
            transactions << transactions_model_.getTransaction(index.row());
            const Transaction& transaction = transactions.back();
            transactionSummary << transaction.date_time.toString(Qt::ISODate) + ": " + transaction.description;
        }
    }
    if (transactions.size() <= 1) {
        QMessageBox::warning(this, "Warning", "Need to select more than one row to merge.", QMessageBox::Ok);
        return;
    }

    QMessageBox warningMsgBox;
    warningMsgBox.setText("Are you sure you want to merge the following transactions?");
    warningMsgBox.setInformativeText(transactionSummary.join("\n"));
    warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    warningMsgBox.setDefaultButton(QMessageBox::Cancel);
    switch (warningMsgBox.exec()) {
    case QMessageBox::Ok: {
        QDate earliest_date(2200, 12, 31);
        if (!book.db.transaction()) {
            qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << book.db.lastError();
            return;
        }
        Transaction merged_transaction;
        for (const Transaction& transaction : transactions) {
            earliest_date = qMin(earliest_date, transaction.date_time.date());
            merged_transaction += transaction;
            if (!book.removeTransaction(transaction.id)) {
                qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m";
                book.db.rollback();
                return;
            }
        }
        if (!book.insertTransaction(user_id, merged_transaction)) {
            qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m";
            book.db.rollback();
            return;
        }
        if (!book.db.commit()) {
            qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << book.db.lastError();
            return;
        }
        financial_statement.getStartStateFor(earliest_date);
        refreshTable();
        break;
    }
    case QMessageBox::Cancel:
        return;
    }
}

void HomeWindow::onPushButtonDeleteClicked() {
    // Get selected transactions.
    QList<Transaction> transactions;
    QStringList transactionSummary;
    for (QModelIndex index : ui->tableView->selectionModel()->selectedIndexes()) {
        if (index.isValid()) {
            transactions << transactions_model_.getTransaction(index.row());
            const Transaction& transaction = transactions.back();
            transactionSummary << transaction.date_time.toString(Qt::ISODate) + ": " + transaction.description;
        }
    }
    if (transactions.empty()) {
        QMessageBox::warning(this, "Warning", "No transaction was selected.", QMessageBox::Ok);
        return;
    }

    QMessageBox warningMsgBox;
    warningMsgBox.setText("Are you sure you want to delete the following transactions?");
    warningMsgBox.setInformativeText(transactionSummary.join("\n"));
    warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    warningMsgBox.setDefaultButton(QMessageBox::Cancel);

    switch (warningMsgBox.exec()) {
    case QMessageBox::Ok:
        for (const Transaction& transaction : transactions) {
            if (!book.removeTransaction(transaction.id)) {
                qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m";
                return;
            }
            financial_statement.getStartStateFor(transaction.date_time.date());
        }
        refreshTable();
        break;
    case QMessageBox::Cancel:
        return;
    }
}

void HomeWindow::on_pushButtonChangeTimeZone_clicked() {
    // Get selected transactions.
    QList<Transaction> transactions;
    QStringList transactionSummary;
    for (QModelIndex index : ui->tableView->selectionModel()->selectedIndexes()) {
        if (index.isValid()) {
            transactions << transactions_model_.getTransaction(index.row());
            const Transaction& transaction = transactions.back();
            transactionSummary << transaction.date_time.toString(Qt::ISODate) + ": " + transaction.description;
        }
    }
    if (transactions.empty()) {
        QMessageBox::warning(this, "Warning", "No transaction was selected.", QMessageBox::Ok);
        return;
    }

    QDialog dialog(this);  // Create the dialog
    // Create a label with informative text
    QLabel *infoLabel = new QLabel("Please set your time zone for:\n" + transactionSummary.join("\n"), &dialog);

    // Create the combo box for time zone selection
    QComboBox *comboBox = new QComboBox(&dialog);
    AddTransaction::setupTimeZoneComboBox(comboBox);

    // Create the dialog button box with "OK" and "Cancel" buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    // Connect the button box signals to the dialog slots
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Create the layout and add the combo box and button box to it
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addWidget(infoLabel);
    layout->addWidget(comboBox);
    layout->addWidget(buttonBox);

    // Set the layout for the dialog
    dialog.setLayout(layout);

    // Execute the dialog and check the result
    if (dialog.exec() == QDialog::Accepted) {
        QTimeZone timeZone(comboBox->currentText().toUtf8());
        for (Transaction transaction : transactions) {
            qDebug() << "Before: " << transaction.date_time;
            transaction.date_time.setTimeZone(timeZone);
            qDebug() << "After: " << transaction.date_time;
            book.insertTransaction(user_id, transaction);
            book.removeTransaction(transaction.id);
        }
        // Handle the selected time zone ID
        qDebug() << "Selected time zone:" << timeZone;
        // You can now use the selected time zone ID as needed
        refreshTable();
        return;
    } else {
        qDebug() << "Time zone selection was canceled.";  // The user canceled the dialog
    }
}

void HomeWindow::onActionAddTransactionTriggered() {
    AddTransaction* addTransaction = new AddTransaction(this);
    addTransaction->setAttribute(Qt::WA_DeleteOnClose);
    addTransaction->show();
}

void HomeWindow::onActionAccountManagerTriggered() {
    AccountManager accountManager(this);

    accountManager.exec();
    initCategoryComboBox();
    refreshTable();
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

void HomeWindow::onActionLoginTriggered() {
    // TODO: Change to username & password login.
    bool ok;
    int id = QInputDialog::getInt(this, tr("QInputDialog::getInteger()"), tr("User ID:"), 0, 1, 100, 1, &ok);
    if (ok) {
        user_id = id;
    }

    book.updateLoginTime(user_id);
    // TODO: Use signal / slot for user_id_ changed event.
    household_manager.model_.setFilter(QString("user_id = %1").arg(user_id));
    initCategoryComboBox();
    refreshTable();
}

void HomeWindow::onActionLogoutTriggered() {
    user_id = -1;

    // TODO: Use signal / slot for user_id_ changed event.
    household_manager.model_.setFilter(QString("user_id = %1").arg(user_id));
    refreshTable();
}
