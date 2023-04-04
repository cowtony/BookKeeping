#include "add_transaction.h"
#include "ui_add_transaction.h"

#include <QDebug>
#include <QMessageBox>

#include "Book.h"
#include "main_window.h"

AddTransaction::AddTransaction(QWidget *parent): QMainWindow(parent), ui_(new Ui::AddTransaction),
    book_(static_cast<MainWindow*>(parent)->book_) {
    ui_->setupUi(this);

    tableMap.insert(Account::Asset,     ui_->tableWidget_Assets);
    tableMap.insert(Account::Expense,   ui_->tableWidget_Expenses);
    tableMap.insert(Account::Revenue,   ui_->tableWidget_Revenues);
    tableMap.insert(Account::Liability, ui_->tableWidget_Liabilities);

    QPalette palette = ui_->tableWidget_Revenues->palette();
    palette.setColor(QPalette::Base, Qt::gray);
    ui_->tableWidget_Revenues->setPalette(palette);
    ui_->tableWidget_Liabilities->setPalette(palette);

    ui_->comboBox_Currency->addItems(Currency::kCurrencyToCode.values());

    initialization();

    connect(this, &AddTransaction::insertTransactionFinished, static_cast<MainWindow*>(parent), &MainWindow::refreshTable);
}

void AddTransaction::initialization() {
  // Date Time
  ui_->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui_->calendarWidget->setSelectedDate(ui_->dateTimeEdit->date());   // maybe not necessary?

  // Repleace Date Time (For replace use only)
  replacedDateTime.setDate(QDate(1990, 05, 25));
  replacedDateTime.setTime(QTime(00, 00, 00));

  // Description
  ui_->lineEdit_Description->clear();

  // Insert Button
  ui_->pushButton_Insert->setText("Insert");

  // Recursive Transaction
  ui_->checkBox_RecursiveTransaction->setChecked(false);
  ui_->dateEdit_nextTransaction->setEnabled(false);

  for (QTableWidget* tableWidget : {ui_->tableWidget_Assets, ui_->tableWidget_Revenues, ui_->tableWidget_Expenses, ui_->tableWidget_Liabilities}) {
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
    connect(add_row, &QPushButton::clicked, [this, tableWidget](){ this->insertTableRow(tableWidget); } );
    tableWidget->setCellWidget(0, 0, add_row);
    tableWidget->cellWidget(0, 0)->setToolTip("Add a new row");
    // Disable all cells on the right of `AddRow` button.
    for (int col = 1; col < tableWidget->columnCount(); ++col) {
      QTableWidgetItem* item = new QTableWidgetItem();
      tableWidget->setItem(0, col, item);
      item->setFlags(item->flags() & ~Qt::ItemIsEnabled);  // set to disable
    }
  }
}

void AddTransaction::setTransaction(const Transaction &transaction) {
  ui_->pushButton_Insert->setText("Replace");

  // Date time
  replacedDateTime = transaction.date_time;
  ui_->dateTimeEdit->setDateTime(transaction.date_time);

  // Description
  if (transaction.description.contains("[R]")) {
    ui_->checkBox_RecursiveTransaction->setChecked(true);
    ui_->dateEdit_nextTransaction->setDate(ui_->calendarWidget->selectedDate().addMonths(1));
  }
  ui_->lineEdit_Description->setText(QString(transaction.description).remove("[R]"));

  for (const Account &account : transaction.getAccounts()) {
    setTableRow(tableMap.value(account.type), account, transaction.getMoneyArray(account));
  }

  show();
}

Transaction AddTransaction::getTransaction() {
    Transaction retTransaction;
    retTransaction.date_time = ui_->dateTimeEdit->dateTime();
    retTransaction.description = ui_->lineEdit_Description->text();

    for (QTableWidget* tableWidget : {ui_->tableWidget_Assets,
                                      ui_->tableWidget_Expenses,
                                      ui_->tableWidget_Revenues,
                                      ui_->tableWidget_Liabilities}) {
        const Account::Type tableType = tableMap.key(tableWidget);

        for (int row = 0; row < tableWidget->rowCount() - 1; row++) {
            QComboBox *cateComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 0));
            QComboBox *nameComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 1));

            if (nameComboBox->currentText().isEmpty()) {
                continue;
            }

            Account account(tableType, cateComboBox->currentText(), nameComboBox->currentText());
            MoneyArray money_array(ui_->dateTimeEdit->date(), book_.queryCurrencyType(account));
            for (int col = 2; col < tableWidget->columnCount(); col++) {
                QLineEdit *line_edit = static_cast<QLineEdit*>(tableWidget->cellWidget(row, col));
                Money money(ui_->dateTimeEdit->date(), line_edit->text(), book_.queryCurrencyType(account));
                if (!line_edit->text().isEmpty()) {
                    line_edit->setText(money);
                    if (money.amount_ < 0) {
                        line_edit->setStyleSheet("color: red");
                    } else {
                        line_edit->setStyleSheet("color: black");
                    }
                }
                money_array.push_back(money);
            }
            if (money_array.sum().amount_ != 0.00) {
                retTransaction.addMoneyArray(account, money_array);
            }
        }
    }
    return retTransaction;
}

void AddTransaction::on_pushButton_Insert_clicked() {
    QStringList errorMsg;

    Transaction t = getTransaction();
    errorMsg << t.validate();

    if (book_.dateTimeExist(ui_->dateTimeEdit->dateTime())) {
        if (ui_->pushButton_Insert->text() != "Replace" or ui_->dateTimeEdit->dateTime() != replacedDateTime) {
            errorMsg << "Date Time already exist.";
        }
    }

    if (!errorMsg.empty()) {
        QMessageBox::warning(this, "Warning!", errorMsg.join('\n'), QMessageBox::Ok);
        return;
    }

    if (ui_->pushButton_Insert->text() == "Replace") {
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
    if (ui_->checkBox_RecursiveTransaction->isChecked()) {
        t.date_time = ui_->dateEdit_nextTransaction->dateTime();
        t.description = "[R]" + ui_->lineEdit_Description->text();
        while (book_.dateTimeExist(t.date_time)) {
            t.date_time = t.date_time.addSecs(1);
        }
        book_.insertTransaction(t, /*ignore_error=*/true);
    }

    emit insertTransactionFinished(this);
    close();
    destroy();
    deleteLater();
}

/************************** Slots *********************************/
void AddTransaction::on_calendarWidget_selectionChanged() {
  ui_->dateTimeEdit->setDate(ui_->calendarWidget->selectedDate());
  ui_->dateEdit_nextTransaction->setDate(ui_->calendarWidget->selectedDate().addMonths(1));
  ui_->label_Currency->setText("Currency: " + QString::number(g_currency.getExchangeRate(ui_->dateTimeEdit->date(), Currency::USD, Currency::CNY)));
}

void AddTransaction::on_dateTimeEdit_dateTimeChanged(const QDateTime& date_time) {  // This may not necessary
  ui_->calendarWidget->setSelectedDate(date_time.date());
}

void AddTransaction::on_lineEdit_Description_editingFinished() {
  ui_->lineEdit_Description->setText(ui_->lineEdit_Description->text().simplified());
}

void AddTransaction::onAccountCateChanged(QTableWidget* tableWidget, int row) {
  QComboBox* cateComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 0));
  QComboBox* nameComboBox = static_cast<QComboBox*>(tableWidget->cellWidget(row, 1));

  nameComboBox->clear();
  QStringList l_accountNamesByDate = book_.queryAccountNamesByLastUpdate(tableMap.key(tableWidget), cateComboBox->currentText(), ui_->dateTimeEdit->dateTime());
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
      lineEdit->setText(money_array.getMoney(col - 2));
      if (money_array.getMoney(col - 2).amount_ < 0) {
        lineEdit->setStyleSheet("color: red");
      } else {
        lineEdit->setStyleSheet("color: black");
      }
  }
}

void AddTransaction::on_checkBox_RecursiveTransaction_stateChanged(int arg1) {
    ui_->dateEdit_nextTransaction->setEnabled(arg1 > 0);
}
