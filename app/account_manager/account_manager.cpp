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
      book_(static_cast<HomeWindow*>(parent)->book),
      user_id_(static_cast<HomeWindow*>(parent)->user_id),
      account_model_(parent) {
    ui->setupUi(this);

    ui->treeView->setModel(&account_model_);
    ui->gridLayout->addWidget(ui->treeView, 0, 0, 1, 2);

    account_model_.setupCategoriesAndAccounts(book_.queryAllCategories(user_id_), book_.queryAllAccounts(user_id_));
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
    AccountTreeNode* node = AccountsModel::getItem(current);
    ui->pushButton_Add   ->setEnabled(node->depth() < 3);  // Can add under account type or category.
    ui->pushButton_Delete->setEnabled(node->depth() == 3 || (node->depth() == 2 && node->childCount() == 0));  // Can delete an account or empty category.
}

void AccountManager::onReceiveErrorMessage(const QString& message) {
    QMessageBox::warning(this, "WARNING", message, QMessageBox::Ok);
}

void AccountManager::on_pushButton_Add_clicked() {
    AccountTreeNode* item = AccountsModel::getItem(ui->treeView->currentIndex());
    switch (item->depth()) {
        case 1: {  // Account type level: Add a category
            bool ok;
            QString category_name = QInputDialog::getText(this, "Add category into table: " + item->name(), "Category:", QLineEdit::Normal, "", &ok);
            if (!ok || category_name.isEmpty()) {
                return;
            }
            // This should not be needed because this category is disabled for selection.
            if (item->accountType() == Account::Revenue && category_name == "Investment") {
                QMessageBox::warning(this, "Insert Failed", "Cannot add Revenue::Investment since 'Investment' is reserved.", QMessageBox::Ok);
                return;
            }
            QSharedPointer<Account> category = book_.insertCategory(user_id_, item->accountType(), category_name);
            if (category) {
                account_model_.appendRow(ui->treeView->currentIndex(), category_name, category);
            } else {
                QMessageBox::warning(this, "Insert Failed", item->name() + ": " + category_name, QMessageBox::Ok);
            }
            return;
        }
        case 2: {  // Category level: Add an account
            bool ok;
            QString account_name = QInputDialog::getText(this, "Add account into category: " + item->name(), "Name:", QLineEdit::Normal, "", &ok);
            if (!ok || account_name.isEmpty()) {
                return;
            }
            QSharedPointer<Account> account = book_.insertAccount(user_id_, item->accountType(), item->accountGroup(), account_name);
            if (account) {
                account_model_.appendRow(ui->treeView->currentIndex(), account_name, account);
                onCurrentItemChanged(ui->treeView->currentIndex(), ui->treeView->currentIndex());
            } else {
                QMessageBox::warning(this, "Insert Failed", item->name() + ": " + account_name, QMessageBox::Ok);
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
    if (item->depth() == 1) {
        QMessageBox::warning(this, "Warning", "Cannot delete the whole account type.", QMessageBox::Ok);
        return;
    } else if (item->depth() == 2) {
        if (!book_.removeCategory(item->account()->categoryId())) {
            QMessageBox::warning(this, "Cannot delete!", "The category still have account(s) under to it!", QMessageBox::Ok);
            return;
        }
    } else if (item->depth() == 3) {
        if (item->account()->isInvestment()) {
            QMessageBox::warning(this, "Cannot delete investment account!", "Please uncheck the investment first.", QMessageBox::Ok);
            return;
        }
        if (!book_.removeAccount(item->account()->accountId())) {
            QMessageBox::warning(this, "Cannot delete!", "The account still have transaction(s) link to it!", QMessageBox::Ok);
            return;
        }
    }
    account_model_.removeItem(ui->treeView->currentIndex());
    onCurrentItemChanged(ui->treeView->currentIndex(), ui->treeView->currentIndex());
}
