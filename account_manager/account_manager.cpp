#include "account_manager.h"
#include "ui_account_manager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QDropEvent>

#include "home_window/home_window.h"
#include "book/book.h"

AccountManager::AccountManager(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::AccountManager),
    book_(static_cast<HomeWindow*>(parent)->book_),
    account_model_(static_cast<HomeWindow*>(parent)->book_) {
    ui->setupUi(this);

    ui->treeView->setModel(&account_model_);
    ui->gridLayout->addWidget(ui->treeView, 0, 0, 1, 2);

    account_model_.setupAccounts(book_.queryAllAccountsFrom({Account::Asset, Account::Liability, Account::Revenue, Account::Expense}));
    ui->treeView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setColumnWidth(0, 200);

    connect(&account_model_, &AccountsModel::errorMessage, this, &AccountManager::onReceiveErrorMessage);
    connect(&account_model_, &AccountsModel::dataChanged,  static_cast<HomeWindow*>(parent), &HomeWindow::refreshTable);
    connect(&account_model_, &AccountsModel::dataChanged,  static_cast<HomeWindow*>(parent), &HomeWindow::setCategoryComboBox);
    connect(&account_model_, &AccountsModel::rowsInserted, static_cast<HomeWindow*>(parent), &HomeWindow::setCategoryComboBox);
    connect(&account_model_, &AccountsModel::rowsMoved,    static_cast<HomeWindow*>(parent), &HomeWindow::setCategoryComboBox);
    connect(&account_model_, &AccountsModel::rowsRemoved,  static_cast<HomeWindow*>(parent), &HomeWindow::setCategoryComboBox);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountManager::onCurrentItemChanged);
}

AccountManager::~AccountManager() {
    delete ui;
}

void AccountManager::onCurrentItemChanged(const QModelIndex& current, const QModelIndex& /* previous */) {
    ui->pushButton_Add   ->setEnabled(AccountsModel::getItem(current)->depth() != 3);  // Can add under type or category.
    ui->pushButton_Delete->setEnabled(AccountsModel::getItem(current)->depth() == 3);  // Can delete meta account.
}

void AccountManager::onReceiveErrorMessage(const QString& message) {
    QMessageBox::warning(this, "WARNING", message, QMessageBox::Ok);
}

void AccountManager::on_pushButton_Add_clicked() {
  AccountTreeNode* item = AccountsModel::getItem(ui->treeView->currentIndex());
  switch (item->depth()) {
    case 1: {  // Account type level
      bool ok;
      QString category_name = QInputDialog::getText(this, "Add category into table: " + item->name(), "Category:", QLineEdit::Normal, "", &ok);
      if (ok and !category_name.isEmpty()) {
        if (item->accountType() == "Revenue" && category_name == "Investment") {
          QMessageBox::warning(this, "Insert Failed", "Cannot add Revenue::Investment since 'Investment' is reserved.", QMessageBox::Ok);
          return;
        }
        if (book_.insertCategory(item->accountType(), category_name)) {
          QModelIndex category_index = account_model_.appendRow(ui->treeView->currentIndex(), category_name);
          account_model_.appendRow(category_index, category_name);
        } else {
          QMessageBox::warning(this, "Insert Failed", item->name() + ": " + category_name, QMessageBox::Ok);
        }
      }
      return;
    }
    case 2: {  // Category level
      bool ok;
      QString account_name = QInputDialog::getText(this, "Add account into category: " + item->name(), "Name:", QLineEdit::Normal, "", &ok);
      if (ok and !account_name.isEmpty()) {
        if (book_.insertAccount(Account(item->accountType(), item->accountGroup(), account_name))) {
          account_model_.appendRow(ui->treeView->currentIndex(), account_name);
        } else {
          QMessageBox::warning(this, "Insert Failed", item->name() + ": " + account_name, QMessageBox::Ok);
        }
      }
      return;
    }
    case 3:
      QMessageBox::warning(this, "Cannot Add!", "Cannot add sub item under account name.", QMessageBox::Ok);
      return;
  }
}

void AccountManager::on_pushButton_Delete_clicked() {
  AccountTreeNode* item = AccountsModel::getItem(ui->treeView->currentIndex());
  if (item->depth() != 3) {
    QMessageBox::warning(this, "Warning", "Cannot delete the whole type or category.", QMessageBox::Ok);
    return;
  }
  if (!book_.removeAccount(*item->account())) {
    QMessageBox::warning(this, "Cannot delete!", "The account still have transactions link to it!", QMessageBox::Ok);
    return;
  }
  account_model_.removeItem(ui->treeView->currentIndex());
}
