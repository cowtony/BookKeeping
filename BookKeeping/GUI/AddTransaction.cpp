#include "AddTransaction.h"
#include "ui_AddTransaction.h"

#include <QDebug>
#include <QMessageBox>

#include "Book.h"

AddTransaction::AddTransaction(Book& book, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AddTransaction), book_(book) {
  ui->setupUi(this);

  tableMap.insert(Account::Asset,     ui->tableWidget_Assets);
  tableMap.insert(Account::Expense,   ui->tableWidget_Expenses);
  tableMap.insert(Account::Revenue,   ui->tableWidget_Revenues);
  tableMap.insert(Account::Liability, ui->tableWidget_Liabilities);

  QPalette palette = ui->tableWidget_Revenues->palette();
  palette.setColor(QPalette::Base, Qt::gray);
  ui->tableWidget_Revenues->setPalette(palette);
  ui->tableWidget_Liabilities->setPalette(palette);

  ui->comboBox_Currency->addItems(Currency::kCurrencyToCode.values());

  initialization();
}

AddTransaction::~AddTransaction() {
  delete ui;
}

void AddTransaction::initialization() {
  // Date Time
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui->calendarWidget->setSelectedDate(ui->dateTimeEdit->date());   // maybe not necessary?

  // Repleace Date Time (For replace use only)
  replacedDateTime.setDate(QDate(1990, 05, 25));
  replacedDateTime.setTime(QTime(00, 00, 00));

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

    tableWidget->insertRow(0);
    tableWidget->setRowHeight(0, 20);
    tableWidget->setColumnWidth(0, 100);
    tableWidget->setColumnWidth(1, 200);
    for (int col = 2; col < tableWidget->columnCount(); col++) {
      tableWidget->setColumnWidth(col, 80);
    }

    QPushButton* addRow = new QPushButton;
    addRow->setText("Add");
    connect(addRow, &QPushButton::clicked, [this, tableWidget](){ this->insertTableRow(tableWidget); } );
    tableWidget->setCellWidget(0, 0, addRow);
    tableWidget->cellWidget(0, 0)->setToolTip("Add a new row");

    for (int col = 1; col <= 3; col++) {
      QTableWidgetItem* item = new QTableWidgetItem();
      tableWidget->setItem(0, col, item);
      item->setFlags(item->flags() & ~Qt::ItemIsEnabled);  // set to disable
    }
  }
}

void AddTransaction::setTransaction(const Transaction &transaction) {
  ui->pushButton_Insert->setText("Replace");

  // Date time
  replacedDateTime = transaction.date_time_;
  ui->dateTimeEdit->setDateTime(transaction.date_time_);

  // Description
  if (transaction.description_.contains("[R]")) {
    ui->checkBox_RecursiveTransaction->setChecked(true);
    ui->dateEdit_nextTransaction->setDate(ui->calendarWidget->selectedDate().addMonths(1));
  }
  ui->lineEdit_Description->setText(QString(transaction.description_).remove("[R]"));

  for (const Account &account : transaction.getAccounts()) {
    setTableRow(tableMap.value(account.type), account, transaction.getMoneyArray(account));
  }

  show();
}

Transaction AddTransaction::getTransaction() {
  Transaction retTransaction;
  retTransaction.date_time_ = ui->dateTimeEdit->dateTime();
  retTransaction.description_ = ui->lineEdit_Description->text();

  for (QTableWidget* tableWidget : {ui->tableWidget_Assets,
                                    ui->tableWidget_Expenses,
                                    ui->tableWidget_Revenues,
                                    ui->tableWidget_Liabilities}) {
    const Account::Type tableType = tableMap.key(tableWidget);

    for (int row = 0; row < tableWidget->rowCount() - 1; row++) {
      QComboBox *cateComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 0));
      QComboBox *nameComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 1));

      if (nameComboBox->currentText().isEmpty()) {
        continue;
      }

      Account account(tableType, cateComboBox->currentText(), nameComboBox->currentText());

      MoneyArray moneyArray(ui->dateTimeEdit->date(), book_.queryCurrencyType(account));
      for (int col = 2; col < tableWidget->columnCount(); col++) {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(tableWidget->cellWidget(row, col));
        Money money(ui->dateTimeEdit->date(), lineEdit->text(), book_.queryCurrencyType(account));
        if (!lineEdit->text().isEmpty()) {
          lineEdit->setText(money.toString());
          if (money.amount_ < 0) {
            lineEdit->setStyleSheet("color: red");
          } else {
            lineEdit->setStyleSheet("color: black");
          }
        }
        moneyArray.push_back(money);
      }
      retTransaction.addMoneyArray(account, moneyArray);
    }
  }
  return retTransaction;
}

void AddTransaction::on_pushButton_Insert_clicked() {
  QStringList errorMsg;

  Transaction t = getTransaction();
  errorMsg << t.validate();

  if (book_.dateTimeExist(ui->dateTimeEdit->dateTime())) {
    if (ui->pushButton_Insert->text() != "Replace" or ui->dateTimeEdit->dateTime() != replacedDateTime) {
      errorMsg << "Date Time already exist.";
    }
  }

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
        book_.removeTransaction(replacedDateTime);
        break;
      case QMessageBox::Cancel:
        return;
    }
  }

  book_.insertTransaction(t);
  if (ui->checkBox_RecursiveTransaction->isChecked()) {
    t.date_time_ = ui->dateEdit_nextTransaction->dateTime();
    t.description_ = "[R]" + ui->lineEdit_Description->text();
    while (book_.dateTimeExist(t.date_time_)) {
      t.date_time_ = t.date_time_.addSecs(1);
    }
    book_.insertTransaction(t);
  }

  emit insertTransactionFinished(this);
  close();
  destroy();
  deleteLater();
}

/************************** Slots *********************************/
void AddTransaction::on_calendarWidget_selectionChanged() {
  ui->dateTimeEdit->setDate(ui->calendarWidget->selectedDate());
  ui->dateEdit_nextTransaction->setDate(ui->calendarWidget->selectedDate().addMonths(1));
  ui->label_Currency->setText("Currency: " + QString::number(g_currency.getExchangeRate(ui->dateTimeEdit->date(), Currency::USD, Currency::CNY)));
}

void AddTransaction::on_dateTimeEdit_dateTimeChanged(const QDateTime &dateTime_) {  // This may not necessary
  ui->calendarWidget->setSelectedDate(dateTime_.date());
}

void AddTransaction::on_lineEdit_Description_editingFinished() {
  ui->lineEdit_Description->setText(ui->lineEdit_Description->text().simplified());
}

void AddTransaction::onAccountCateChanged(QTableWidget* tableWidget, int row) {
  QComboBox* cateComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 0));
  QComboBox* nameComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 1));

  nameComboBox->clear();
  QStringList l_accountNamesByDate = book_.queryAccountNamesByLastUpdate(tableMap.key(tableWidget), cateComboBox->currentText(), ui->dateTimeEdit->dateTime());
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
  for (QTableWidget* tableWidget : tableMap.values()) {
    for (int row = 0; row < tableWidget->rowCount() - 1; row++) {
      for (int col = 2; col < tableWidget->columnCount(); col++) {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(tableWidget->cellWidget(row, col));
        if (lineEdit->text().isEmpty()) {
          lastNode = Node(lineEdit, tableMap.key(tableWidget));
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
    split.changeCurrency(book_.queryCurrencyType(lastNode));
    lastNode.m_lineEdit->setText(split.toString());
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
  cateComboBox->addItems(book_.queryCategories(tableMap.key(tableWidget)));
  return row;
}

void AddTransaction::setTableRow(QTableWidget *table_widget, Account account, const MoneyArray &money_array) {
  int row = insertTableRow(table_widget);

  QComboBox *cateComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 0));
  cateComboBox->setCurrentText(account.category);
  QComboBox *nameComboBox = static_cast<QComboBox*>(table_widget->cellWidget(row, 1));
  nameComboBox->setCurrentText(account.name);

  for (int col = 2; col < table_widget->columnCount(); col++) {
      QLineEdit *lineEdit = static_cast<QLineEdit*>(table_widget->cellWidget(row, col));
      lineEdit->setText(money_array.getMoney(col - 2).toString());
      if (money_array.getMoney(col - 2).amount_ < 0) {
        lineEdit->setStyleSheet("color: red");
      } else {
        lineEdit->setStyleSheet("color: black");
      }
  }
}

void AddTransaction::on_checkBox_RecursiveTransaction_stateChanged(int arg1) {
  ui->dateEdit_nextTransaction->setEnabled(arg1 > 0);
}
