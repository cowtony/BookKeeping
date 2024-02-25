#include "account_manager.h"

#include <QBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QDropEvent>

#include "home_window/home_window.h"
#include "book/book.h"

AccountManager::AccountManager(QWidget* parent)
    : QDialog(parent),
      book_(static_cast<HomeWindow*>(parent)->book),
      user_id_(static_cast<HomeWindow*>(parent)->user_id),
      account_model_(parent) {

    this->setWindowTitle("Account Manager");
    account_model_.setupCategoriesAndAccounts(book_.queryAllCategories(user_id_), book_.queryAllAccounts(user_id_));
    connect(&account_model_, &AccountsModel::errorMessage, this, &AccountManager::onReceiveErrorMessage);

    // Create the tree view
    treeView_ = new QTreeView(this);
    treeView_->setModel(&account_model_);
    treeView_->setDragDropMode(QAbstractItemView::InternalMove);
    treeView_->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView_->setDropIndicatorShown(true);
    treeView_->setColumnWidth(0, 200);
    connect(treeView_->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountManager::onCurrentItemChanged);

    // Create the add button
    pushButton_Add_ = new QPushButton("Add", this);
    connect(pushButton_Add_, &QPushButton::clicked, this, &AccountManager::on_pushButton_Add_clicked);

    // Create the delete button
    pushButton_Delete_ = new QPushButton("Delete", this);
    connect(pushButton_Delete_, &QPushButton::clicked, this, &AccountManager::on_pushButton_Delete_clicked);

    // Create a layout for the buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(pushButton_Add_);
    buttonLayout->addStretch(); // This will push the buttons to the left and right
    buttonLayout->addWidget(pushButton_Delete_);
    // Create a main layout and add widgets to it
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(treeView_);
    mainLayout->addLayout(buttonLayout);
    // Set the dialog's layout
    setLayout(mainLayout);
    setMinimumSize(600, 400);
}

AccountManager::~AccountManager() {
}

void AccountManager::onCurrentItemChanged(const QModelIndex& current, const QModelIndex& /* previous */) {
    AccountTreeNode* node = AccountsModel::getItem(current);
    pushButton_Add_   ->setEnabled(node->depth() < 3);  // Can add under account type or category.
    pushButton_Delete_->setEnabled(node->depth() == 3 || (node->depth() == 2 && node->childCount() == 0));  // Can delete an account or empty category.
}

void AccountManager::onReceiveErrorMessage(const QString& message) {
    QMessageBox::warning(this, "WARNING", message, QMessageBox::Ok);
}

void AccountManager::on_pushButton_Add_clicked() {
    AccountTreeNode* item = AccountsModel::getItem(treeView_->currentIndex());
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
                account_model_.appendRow(treeView_->currentIndex(), category_name, category);
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
                account_model_.appendRow(treeView_->currentIndex(), account_name, account);
                onCurrentItemChanged(treeView_->currentIndex(), treeView_->currentIndex());
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
    AccountTreeNode* item = AccountsModel::getItem(treeView_->currentIndex());
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
    account_model_.removeItem(treeView_->currentIndex());
    onCurrentItemChanged(treeView_->currentIndex(), treeView_->currentIndex());
}
