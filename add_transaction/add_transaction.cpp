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
    for (const auto& [account_type, categories] : transaction.data_.asKeyValueRange()) {
        for (const auto& [category, accounts] : categories.asKeyValueRange()) {
            for (const auto& [account, households] : accounts.asKeyValueRange()) {
                setTableRow(table_widgets_.value(account_type), Account(account_type, category, account), households);
            }
        }
    }

    show();
}

Transaction AddTransaction::getTransaction() {
    Transaction transaction;
    transaction.date_time = ui->dateTimeEdit->dateTime();
    transaction.description = ui->lineEdit_Description->text();

    for (QTableWidget* table_widget : {ui->tableWidget_Assets, ui->tableWidget_Expenses, ui->tableWidget_Revenues, ui->tableWidget_Liabilities}) {
        const Account::Type account_type = table_widgets_.key(table_widget);

        for (int row = 0; row < table_widget->rowCount() - 1; ++row) {
            QComboBox *cateComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 0));
            QComboBox *nameComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 1));

            if (nameComboBox->currentText().isEmpty()) {
                continue;
            }

            Account account(account_type, cateComboBox->currentText(), nameComboBox->currentText());
            for (int col = 2; col < table_widget->columnCount(); col++) {
                QLineEdit *line_edit = static_cast<QLineEdit*>(table_widget->cellWidget(row, col));
                QString household = table_widget->horizontalHeaderItem(col)->text();
                Money money(ui->dateTimeEdit->date(), line_edit->text(), book_.queryCurrencyType(user_id_, account));
                if (!line_edit->text().isEmpty()) {
                    line_edit->setText(money);
                    if (money.amount_ < 0) {
                        line_edit->setStyleSheet("color: red");
                    } else {
                        line_edit->setStyleSheet("color: black");
                    }
                }
                transaction.data_[account_type][cateComboBox->currentText()][nameComboBox->currentText()][household] += money;
            }
        }
    }
    return transaction;
}

void AddTransaction::on_pushButton_Insert_clicked() {
    QStringList errorMsg;

    Transaction t = getTransaction();
    errorMsg << t.validate();

    if (!errorMsg.empty()) {
        QMessageBox::warning(this, "Warning!", errorMsg.join('\n'), QMessageBox::Ok);
        return;
    }

    if (ui->pushButton_Insert->text() == "Replace") {
        QMessageBox warningMsgBox;
        warningMsgBox.setText("You are trying to replace a transaction");
        warningMsgBox.setInformativeText("The action cannot be undone, are you sure?");
        warningMsgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        warningMsgBox.setDefaultButton(QMessageBox::Cancel);
        switch ( warningMsgBox.exec()) {
            case QMessageBox::Ok:
                book_.removeTransaction(transaction_id_);
                break;
            case QMessageBox::Cancel:
                return;
        }
    }

    book_.insertTransaction(user_id_, t);
    if (ui->checkBox_RecursiveTransaction->isChecked()) {
        t.date_time = ui->dateEdit_nextTransaction->dateTime();
        t.description = "[R]" + ui->lineEdit_Description->text();
        book_.insertTransaction(user_id_, t, /*ignore_error=*/true);
    }

    emit insertTransactionFinished(this);
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
  QStringList l_accountNamesByDate = book_.queryAccountNamesByLastUpdate(user_id_, table_widgets_.key(tableWidget), cateComboBox->currentText(), ui->dateTimeEdit->dateTime());
  nameComboBox->addItems(l_accountNamesByDate);
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
  struct Node : public Account {
    Node(QLineEdit* p_lineEdit = nullptr, Type p_tableType = Asset)
    : Account(p_tableType, "", ""), m_lineEdit(p_lineEdit) {
      m_sign = type == Asset || type == Expense? 1 : -1;
    }
    int m_sign;
    QLineEdit* m_lineEdit;
  } lastNode;

  int count = 0;
  for (QTableWidget* tableWidget : table_widgets_.values()) {
    for (int row = 0; row < tableWidget->rowCount() - 1; row++) {
      for (int col = 2; col < tableWidget->columnCount(); col++) {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(tableWidget->cellWidget(row, col));
        if (lineEdit->text().isEmpty()) {
          lastNode = Node(lineEdit, table_widgets_.key(tableWidget));
          lastNode.category    = static_cast<QComboBox*>(tableWidget->cellWidget(row, 0))->currentText();
          lastNode.name        = static_cast<QComboBox*>(tableWidget->cellWidget(row, 1))->currentText();
          count++;
        }
      }
    }
  }

  if (count > 0) {
    Transaction t = getTransaction();
    Money remain = - t.getCheckSum();
    Money split = (remain / (lastNode.m_sign * count));
    split.changeCurrency(book_.queryCurrencyType(user_id_, lastNode));
    lastNode.m_lineEdit->setText(split);
    if (split.amount_ < 0) {
      lastNode.m_lineEdit->setStyleSheet("color: red");
    } else {
      lastNode.m_lineEdit->setStyleSheet("color: black");
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
  QComboBox* nameComboBox = new QComboBox;
  tableWidget->setCellWidget(row, 1, nameComboBox);
  for (int col = 2; col < tableWidget->columnCount(); col++) {
    QLineEdit* lineEdit = new QLineEdit;
    tableWidget->setCellWidget(row, col, lineEdit);
    lineEdit->setAlignment(Qt::AlignRight);
    lineEdit->setDisabled(true);
  }
  connect(cateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, tableWidget, row](){ this->onAccountCateChanged(tableWidget, row); });
  cateComboBox->addItem("");
  cateComboBox->addItems(book_.queryCategories(user_id_, table_widgets_.key(tableWidget)));
  return row;
}

void AddTransaction::setTableRow(QTableWidget* table_widget, const Account& account, const QHash<QString, Money>& households) {
    int row = insertTableRow(table_widget);

    QComboBox *cateComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 0));
    cateComboBox->setCurrentText(account.category);
    QComboBox *nameComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 1));
    nameComboBox->setCurrentText(account.name);

    for (const auto& [household, money] : households.asKeyValueRange()) {
        int col = household_to_column_.value(household);
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
