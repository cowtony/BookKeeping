#include "add_transaction.h"
#include "ui_add_transaction.h"

#include <QDebug>
#include <QMessageBox>

#include "book/book.h"
#include "home_window/home_window.h"

AddTransaction::AddTransaction(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::AddTransaction),
      book_(static_cast<HomeWindow*>(parent)->book),
      user_id_(static_cast<HomeWindow*>(parent)->user_id) {
    ui->setupUi(this);

    table_widgets_.insert(Account::Asset,     ui->tableWidget_Assets);
    table_widgets_.insert(Account::Expense,   ui->tableWidget_Expenses);
    table_widgets_.insert(Account::Revenue,   ui->tableWidget_Revenues);
    table_widgets_.insert(Account::Liability, ui->tableWidget_Liabilities);

    QPalette palette = ui->tableWidget_Revenues->palette();
    palette.setColor(QPalette::Base, Qt::gray);
    ui->tableWidget_Revenues->setPalette(palette);
    ui->tableWidget_Liabilities->setPalette(palette);

    ui->comboBox_Currency->addItems(Currency::kCurrencyToCode.values());

    initialization();

    connect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this, &AddTransaction::onCalendarWidgetSelectionChanged);
    connect(ui->dateTimeEdit,   &QDateTimeEdit::dateTimeChanged,    this, &AddTransaction::onDateTimeEditDateTimeChanged);
    connect(this, &AddTransaction::insertTransactionFinished, static_cast<HomeWindow*>(parent), &HomeWindow::refreshTable);
    connect(this, &AddTransaction::insertTransactionFinished, &static_cast<HomeWindow*>(parent)->financial_statement, &FinancialStatement::getStartStateFor);
}

AddTransaction::~AddTransaction() {
    delete ui;
}

void AddTransaction::initialization() {
    // Date Time
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->calendarWidget->setSelectedDate(ui->dateTimeEdit->date());   // maybe not necessary?

    // Description
    ui->lineEdit_Description->clear();

    // Insert Button
    ui->pushButton_Insert->setText("Insert");

    // Recursive Transaction
    ui->checkBox_RecursiveTransaction->setChecked(false);
    ui->dateEdit_nextTransaction->setEnabled(false);

    for (QTableWidget* tableWidget : {ui->tableWidget_Assets, ui->tableWidget_Revenues, ui->tableWidget_Expenses, ui->tableWidget_Liabilities}) {
        while (tableWidget->rowCount() > 0) {
            tableWidget->removeRow(0);
        }
        // Set cell width & height.
        tableWidget->insertRow(0);
        tableWidget->setRowHeight(0, 20);
        tableWidget->setColumnWidth(0, 100);
        tableWidget->setColumnWidth(1, 200);
        for (int col = 2; col < tableWidget->columnCount(); ++col) {
            tableWidget->setColumnWidth(col, 80);
        }

        QPushButton* add_row = new QPushButton;
        add_row->setText("Add");
        connect(add_row, &QPushButton::clicked, this, [this, tableWidget](){ insertTableRow(tableWidget); });
        tableWidget->setCellWidget(0, 0, add_row);
        tableWidget->cellWidget(0, 0)->setToolTip("Add a new row");
        // Disable all cells on the right of `AddRow` button.
        for (int col = 1; col < tableWidget->columnCount(); ++col) {
            QTableWidgetItem* item = new QTableWidgetItem();
            tableWidget->setItem(0, col, item);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);  // set to disable
        }
    }

    // Set the household columns for Revenue & Expense.
    // TODO: Create a Add button and dropdown menu for more flexibility.
    QStringList households = book_.getHouseholds(user_id_);
    for (QTableWidget* tableWidget : {ui->tableWidget_Revenues, ui->tableWidget_Expenses}) {
        tableWidget->setColumnCount(2 + households.size());
        tableWidget->setHorizontalHeaderLabels(QStringList() << "Category" << "Account" << households);
        for (int i = 0; i < households.size(); ++i) {
            tableWidget->setColumnWidth(i + 2, 60);
        }
    }
    for (int i = 0; i < households.size(); ++i) {
        household_to_column_[households[i]] = i + 2;
    }
}

void AddTransaction::setTransaction(const Transaction& transaction) {
    ui->pushButton_Insert->setText("Replace");

    transaction_id_ = transaction.id;
    // Date time
    ui->dateTimeEdit->setDateTime(transaction.date_time);

    // Description
    if (transaction.description.contains("[R]")) {
        ui->checkBox_RecursiveTransaction->setChecked(true);
        ui->dateEdit_nextTransaction->setDate(ui->calendarWidget->selectedDate().addMonths(1));
    }
    ui->lineEdit_Description->setText(QString(transaction.description).remove("[R]"));

    // Data
    for (const auto& [account, household_money] : transaction.getAccounts()) {
        setTableRow(table_widgets_.value(account->accountType()), *account, household_money);
    }

    show();
}

Transaction AddTransaction::getTransaction() {
    Transaction transaction;
    transaction.date_time = ui->dateTimeEdit->dateTime();
    transaction.description = ui->lineEdit_Description->text();

    for (QTableWidget* table_widget : {ui->tableWidget_Assets, ui->tableWidget_Expenses, ui->tableWidget_Revenues, ui->tableWidget_Liabilities}) {
        for (int row = 0; row < table_widget->rowCount() - 1; ++row) {
            QComboBox *nameComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 1));

            if (nameComboBox->currentText().isEmpty()) {
                continue;
            }

            auto account = nameComboBox->currentData().value<QSharedPointer<Account>>();
            Q_ASSERT(account);
            HouseholdMoney household_money(ui->dateTimeEdit->date(), account->currencyType());
            for (int col = 2; col < table_widget->columnCount(); col++) {
                QLineEdit *line_edit = static_cast<QLineEdit*>(table_widget->cellWidget(row, col));
                QString household = account->getFinancialStatementName() == "Balance Sheet"? "All" : table_widget->horizontalHeaderItem(col)->text();
                Money money(ui->dateTimeEdit->date(), line_edit->text(), account->currencyType());
                if (!line_edit->text().isEmpty()) {
                    line_edit->setText(money);
                    if (money.amount_ < 0) {
                        line_edit->setStyleSheet("color: red");
                    } else {
                        line_edit->setStyleSheet("color: black");
                    }
                }
                transaction.addMoney(account, household, money);
            }
        }
    }
    return transaction;
}

void AddTransaction::on_pushButton_Insert_clicked() {
    QStringList errorMsg;

    Transaction transaction = getTransaction();
    errorMsg << transaction.validate();

    if (!errorMsg.empty()) {
        QMessageBox::warning(this, "Warning!", errorMsg.join('\n'), QMessageBox::Ok);
        return;
    }
    QDate earliest_date(2200, 12, 31);
    if (transaction_id_ > 0) {  // This will be a Replace action.
        QMessageBox warningMsgBox;
        warningMsgBox.setText("You are trying to replace a transaction");
        warningMsgBox.setInformativeText("The action cannot be undone, are you sure?");
        warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        warningMsgBox.setDefaultButton(QMessageBox::Cancel);
        switch ( warningMsgBox.exec()) {
        case QMessageBox::Ok:
            earliest_date = qMin(earliest_date, book_.getTransaction(transaction_id_).date_time.date());
            book_.removeTransaction(transaction_id_);
            break;
        case QMessageBox::Cancel:
            return;
        }
    }

    book_.insertTransaction(user_id_, transaction);
    if (ui->checkBox_RecursiveTransaction->isChecked()) {
        transaction.date_time = ui->dateEdit_nextTransaction->dateTime();
        transaction.description = "[R]" + ui->lineEdit_Description->text();
        book_.insertTransaction(user_id_, transaction, /*ignore_error=*/true);
        earliest_date = qMin(earliest_date, transaction.date_time.date());
    }

    emit insertTransactionFinished(earliest_date);
    close();
    destroy();
    deleteLater();
}

/************************** Slots *********************************/
void AddTransaction::onCalendarWidgetSelectionChanged() {
    ui->dateTimeEdit->setDate(ui->calendarWidget->selectedDate());
    ui->dateEdit_nextTransaction->setDate(ui->calendarWidget->selectedDate().addMonths(1));
    ui->label_Currency->setText("Currency: " + QString::number(g_currency.getExchangeRate(ui->dateTimeEdit->date(), Currency::USD, Currency::CNY)));
}

void AddTransaction::onDateTimeEditDateTimeChanged(const QDateTime& date_time) {  // This may not necessary
    ui->calendarWidget->setSelectedDate(date_time.date());
}

void AddTransaction::on_lineEdit_Description_editingFinished() {
    ui->lineEdit_Description->setText(ui->lineEdit_Description->text().simplified());
}

void AddTransaction::onAccountCateChanged(QTableWidget* tableWidget, int row) {
    QComboBox* cateComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 0));
    QComboBox* nameComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 1));

    nameComboBox->clear();
    QList<QSharedPointer<Account>> accounts_by_date = book_.queryAccountNamesByLastUpdate(user_id_, table_widgets_.key(tableWidget), cateComboBox->currentText(), ui->dateTimeEdit->dateTime());
    for (QSharedPointer<Account> account_ptr : accounts_by_date) {
        nameComboBox->addItem(account_ptr->accountName(), QVariant::fromValue(account_ptr));
    }
    nameComboBox->setDisabled(cateComboBox->currentIndex() == 0);

    for (int col = 2; col < tableWidget->columnCount(); col++) {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(tableWidget->cellWidget(row, col));
        if (cateComboBox->currentIndex() == 0) {
            lineEdit->setText("0.00");
            lineEdit->setEnabled(false);
        } else {
            lineEdit->setEnabled(true);
        }
    }

    if (cateComboBox->currentIndex() != 0) { // Set Disable
        gotoStart:
        for (int r = 0; r < tableWidget->rowCount() - 1; r++) {
            if (row == r) continue;
            QComboBox* nameCB = static_cast<QComboBox*>(tableWidget->cellWidget(r, 1));
            if (nameComboBox->currentText() == nameCB->currentText()) {
                if (nameComboBox->currentIndex() < nameComboBox->count() - 1) {
                    nameComboBox->setCurrentIndex(nameComboBox->currentIndex() + 1);
                    goto gotoStart;
                } else {
                    return;
                }
            }
        }
    }

    getTransaction();
}

// Recursivly fill the blank spots.
void AddTransaction::on_pushButton_Split_clicked() {
    struct Node {
        Node(QLineEdit* line = nullptr, Account::Type account_type = Account::Asset)
            : line_edit(line), account_type_(account_type) {  // TODO: The account ID could be retrived here.
            sign = account_type_ == Account::Asset || account_type_ == Account::Expense? 1 : -1;
        }

        int sign;
        QLineEdit* line_edit;
        Account::Type account_type_;
        QString category_name_;
        QString account_name_;
    } last_node;

    int count = 0;
    for (QTableWidget* tableWidget : table_widgets_.values()) {
        for (int row = 0; row < tableWidget->rowCount() - 1; row++) {
            for (int col = 2; col < tableWidget->columnCount(); col++) {
                QLineEdit *line_edit = static_cast<QLineEdit*>(tableWidget->cellWidget(row, col));
                if (line_edit->text().isEmpty()) {
                    last_node = Node(line_edit, table_widgets_.key(tableWidget));
                    last_node.category_name_    = static_cast<QComboBox*>(tableWidget->cellWidget(row, 0))->currentText();
                    last_node.account_name_        = static_cast<QComboBox*>(tableWidget->cellWidget(row, 1))->currentText();
                    count++;
                }
            }
        }
    }

    if (count > 0) {
        Transaction t = getTransaction();
        Money remain = - t.getCheckSum();
        Money split = (remain / (last_node.sign * count));
        split.changeCurrency(book_.queryCurrencyType(user_id_, last_node.account_type_, last_node.category_name_, last_node.account_name_));
        last_node.line_edit->setText(split);
        if (split.amount_ < 0) {
            last_node.line_edit->setStyleSheet("color: red");
        } else {
            last_node.line_edit->setStyleSheet("color: black");
        }
        on_pushButton_Split_clicked();
    }

    return;
}

int AddTransaction::insertTableRow(QTableWidget* tableWidget) {
    int row = tableWidget->rowCount() - 1;
    tableWidget->insertRow(row);
    tableWidget->setRowHeight(row, 20);

    QComboBox* cateComboBox = new QComboBox;
    tableWidget->setCellWidget(row, 0, cateComboBox);
    QComboBox* nameComboBox = new QComboBox;  // TODO: Add user data: QSharedPointer<Account> to the comboBox.
    tableWidget->setCellWidget(row, 1, nameComboBox);
    for (int col = 2; col < tableWidget->columnCount(); col++) {
        QLineEdit* lineEdit = new QLineEdit;
        tableWidget->setCellWidget(row, col, lineEdit);
        lineEdit->setAlignment(Qt::AlignRight);
        lineEdit->setDisabled(true);
    }
    connect(cateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, tableWidget, row](){ onAccountCateChanged(tableWidget, row); });
    cateComboBox->addItem("");
    cateComboBox->addItems(book_.queryCategories(user_id_, table_widgets_.key(tableWidget)));
    return row;
}

void AddTransaction::setTableRow(QTableWidget* table_widget, const Account& account, const HouseholdMoney& household_money) {
    int row = insertTableRow(table_widget);

    QComboBox *cateComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 0));
    cateComboBox->setCurrentText(account.categoryName());
    QComboBox *nameComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 1));
    nameComboBox->setCurrentText(account.accountName());

    for (const auto& [household, money] : household_money.data().asKeyValueRange()) {
        int col = account.getFinancialStatementName() == "Balance Sheet"? 2 : household_to_column_.value(household);
        QLineEdit *lineEdit = static_cast<QLineEdit*>(table_widget->cellWidget(row, col));
        lineEdit->setText(money);
        if (money.amount_ < 0) {
            lineEdit->setStyleSheet("color: red");
        } else {
            lineEdit->setStyleSheet("color: black");
        }
    }
}

void AddTransaction::on_checkBox_RecursiveTransaction_stateChanged(int arg1) {
  ui->dateEdit_nextTransaction->setEnabled(arg1 > 0);
}
