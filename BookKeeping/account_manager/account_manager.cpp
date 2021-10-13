#include "account_manager.h"
#include "ui_AccountManager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QDropEvent>

#include "book.h"

TreeWidget::TreeWidget(Book& book, QWidget *parent)
    : QTreeWidget(parent), book_(book) {
  setSelectionMode(QAbstractItemView::SingleSelection);
  setDragDropMode(QAbstractItemView::InternalMove);
  setDropIndicatorShown(true);
  setAcceptDrops(true);
}

void TreeWidget::dragEnterEvent(QDragEnterEvent *event) {
  drag_from_.clear();
  QTreeWidgetItem *item = currentItem();
  while (item != nullptr) {
    drag_from_.push_front(item->text(0));
    item = item->parent();
  }
  QTreeWidget::dragEnterEvent(event);
}

void TreeWidget::dropEvent(QDropEvent *event) {
  QModelIndex index = indexAt(event->position().toPoint());
  if (!index.isValid()) {  // just in case
    event->setDropAction(Qt::IgnoreAction);  // This may not necessary
    return;
  }

  QTreeWidgetItem* item = itemFromIndex(index);
  QStringList dragTo;

  while (item != nullptr) {
    dragTo.push_front(item->text(0));
    item = item->parent();
  }

  if (drag_from_.size() != 3 || dragTo.size() != 3) {
    return;
  } else if (drag_from_.at(0) != dragTo.at(0)) {
    return;
  } else if (drag_from_.at(1) != dragTo.at(1)) {
    if (book_.accountExist(Account(dragTo.at(0), dragTo.at(1), drag_from_.at(2)))) {
      QMessageBox messageBox;
      messageBox.setText("Do you want to merge with existing account?");
      messageBox.setInformativeText("Old Category: " + drag_from_.at(1) + ", New Category: " + dragTo.at(1));
      messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
      messageBox.setDefaultButton(QMessageBox::Ok);
      switch (messageBox.exec()) {
      case QMessageBox::Ok:
        QApplication::setOverrideCursor(Qt::WaitCursor);
        if (book_.moveAccount(Account(drag_from_.at(0), drag_from_.at(1), drag_from_.at(2)),
                               Account(dragTo.at(0),   dragTo.at(1), drag_from_.at(2)))) {
          delete currentItem();
        }
        QApplication::restoreOverrideCursor();
        return;
      case QMessageBox::Discard:
        return;
      }
    } else {
      QMessageBox messageBox;
      messageBox.setText("You are trying to move item '" + drag_from_.at(2)
                         + "' from '" + drag_from_.at(1)
                         + "' to '" + dragTo.at(1) + "'.");
      messageBox.setInformativeText("Do you want to continue?");
      messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
      messageBox.setDefaultButton(QMessageBox::Ok);
      switch (messageBox.exec()) {
      case QMessageBox::Ok:
        QApplication::setOverrideCursor(Qt::WaitCursor);
        book_.moveAccount(Account(drag_from_.at(0), drag_from_.at(1), drag_from_.at(2)),
                           Account(dragTo.at(0),   dragTo.at(1), drag_from_.at(2)));
        QApplication::restoreOverrideCursor();
        break;
      case QMessageBox::Discard:
        return;
      }
    }

    qDebug() << drag_from_ << dragTo;
  }
  QTreeWidget::dropEvent(event);
}

AccountManager::AccountManager(Book& book, QWidget *parent)
    : QMainWindow(parent), ui_(new Ui::AccountManager), book_(book) {
  ui_->setupUi(this);
  tree_widget_ = new TreeWidget(book_);
  tree_widget_->headerItem()->setText(0, QApplication::translate("AccountManager", "Account Name", nullptr));
  ui_->gridLayout->addWidget(tree_widget_, 0, 0, 1, 2);
  connect(tree_widget_, &QTreeWidget::currentItemChanged, this, &AccountManager::onTreeWidgetItemChanged);
  for (const Account::Type &tableType : {Account::Asset, Account::Liability, Account::Expense, Account::Revenue}) {
    QTreeWidgetItem* accountTypeItem = new QTreeWidgetItem(tree_widget_);
    accountTypeItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
    accountTypeItem->setText(0, Account::kTableName.value(tableType));
    accountTypeItem->setFont(0, kTypeFont);

    for (const QString &category : book_.queryCategories(tableType)) {
      QTreeWidgetItem* accountCategoryItem = new QTreeWidgetItem(accountTypeItem);
      accountCategoryItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled) & ~Qt::ItemIsDragEnabled);
      accountCategoryItem->setText(0, category);
      accountCategoryItem->setFont(0, kCategoryFont);

      for (const QString &name : book_.queryAccountNames(tableType, category)) {
        QTreeWidgetItem* accountItem = new QTreeWidgetItem(accountCategoryItem);
        accountItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
        accountItem->setText(0, name);
        accountItem->setFont(0, kAccountFont);
      }
    }
  }

  // WIP: TreeView
//  account_model_.setupAccounts(book_.queryAllAccountsFrom({Account::Asset, Account::Liability, Account::Revenue, Account::Expense}));
//  ui_->treeView_accounts->setModel(&account_model_);
//  ui_->treeView_accounts->setDragDropMode(QAbstractItemView::InternalMove);
//  ui_->treeView_accounts->setSelectionMode(QAbstractItemView::SingleSelection);
//  ui_->treeView_accounts->setDropIndicatorShown(true);
}

AccountManager::~AccountManager() {
//  delete ui;
}

void AccountManager::onTreeWidgetItemChanged(QTreeWidgetItem *current) {
  QTreeWidgetItem* node = current;
  names_.clear();
  while (node != nullptr) {
    names_.push_front(node->text(0));
    node = node->parent();
  }
  ui_->pushButton_Add   ->setEnabled(names_.size() == 1 or names_.size() == 2);  // Can add under type & category
  ui_->pushButton_Rename->setEnabled(names_.size() == 2 or names_.size() == 3);  // Can rename category & account
  ui_->pushButton_Delete->setEnabled(names_.size() == 3);  // Can only delete account
}

void AccountManager::on_pushButton_Add_clicked() {
  switch (names_.size()) {
    case 1: {  // Account type level
      bool ok;
      QString category_name = QInputDialog::getText(this, "Add category into table: " + names_.back(), "Category:", QLineEdit::Normal, "", &ok);
      if (ok) {
        if (book_.insertCategory(names_.at(0), category_name)) {
          QTreeWidgetItem* category_item = new QTreeWidgetItem(tree_widget_->currentItem());
          category_item->setText(0, category_name);
          category_item->setFont(0, kCategoryFont);
          QTreeWidgetItem* account_item = new QTreeWidgetItem(category_item);  // Insert the dummy account name item with the same name.
          account_item->setText(0, category_name);
          account_item->setFont(0, kAccountFont);
        } else {
          QMessageBox::warning(this, "Insert Failed", names_.back() + ": " + category_name, QMessageBox::Ok);
        }
      }
      return;
    }
    case 2: {  // Category level
      bool ok;
      QString account_name = QInputDialog::getText(this, "Add account into category: " + names_.back(), "Name:", QLineEdit::Normal, "", &ok);
      if (ok && !account_name.isEmpty()) {
        if (book_.insertAccount(Account(names_.at(0), names_.at(1), account_name))) {
          QTreeWidgetItem* account_item = new QTreeWidgetItem(tree_widget_->currentItem());
          account_item->setText(0, account_name);
          account_item->setFont(0, kAccountFont);
        } else {
          QMessageBox::warning(this, "Insert Failed", names_.back() + ": " + account_name, QMessageBox::Ok);
        }
      }
      return;
    }
    case 3:
      QMessageBox::warning(this, "Cannot Add!", "Cannot add sub item under account name.", QMessageBox::Ok);
      return;
  }
}

void AccountManager::on_pushButton_Rename_clicked() {
  switch (names_.size()) {
  case 1:  // Type level
    QMessageBox::warning(this, "Warning", "Cannot rename account type.", QMessageBox::Ok);
    return;
  case 2: {  // Category level
    bool ok;
    QString new_category_name = QInputDialog::getText(this, "Rename Category", "Name:", QLineEdit::Normal, names_.back(), &ok);
    if (!ok or new_category_name.isEmpty()) {
      return;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    bool success = book_.renameCategory(names_.at(0), names_.at(1), new_category_name);
    QApplication::restoreOverrideCursor();
    if (success) {
      tree_widget_->currentItem()->setText(0, new_category_name);
      names_[1] = new_category_name;
      emit categoryChanged();
    } else {
      QMessageBox::warning(this, "Warning", "Cannot rename account type.", QMessageBox::Ok);
    }
    return;
  }
  case 3: { // Account Level
    bool ok;
    QString new_account_name = QInputDialog::getText(this, "Rename Account", "Name:", QLineEdit::Normal, names_.back(), &ok);
    if (!ok or new_account_name.isEmpty()) {
      return;
    }
    Account old_account(names_.at(0), names_.at(1), names_.at(2));
    Account new_account(names_.at(0), names_.at(1), new_account_name);

    if (book_.accountExist(new_account)) {  // new_account_name exist, perform merge.
      QMessageBox messageBox;
      messageBox.setText("Do you want to merge with existing account?");
      messageBox.setInformativeText("OldName: " + names_.at(2) + ", NewName: " + new_account_name);
      messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
      messageBox.setDefaultButton(QMessageBox::Ok);
      switch (messageBox.exec()) {
        case QMessageBox::Ok:
          QApplication::setOverrideCursor(Qt::WaitCursor);
          if (book_.moveAccount(old_account, new_account)) {
            delete tree_widget_->currentItem();
            onTreeWidgetItemChanged(tree_widget_->currentItem());
            emit accountNameChanged();
          }
          QApplication::restoreOverrideCursor();
          return;
        case QMessageBox::Discard:
          return;
      }
    } else if (book_.moveAccount(old_account, new_account)) {  // Simply rename.
      tree_widget_->currentItem()->setText(0, new_account_name);
      names_[2] = new_account_name;
      emit accountNameChanged();
    }
    return;
  }
  }
}

void AccountManager::on_pushButton_Delete_clicked() {
  if (names_.size() != 3) {
    QMessageBox::warning(this, "Warning", "Cannot delete the whole type or category.", QMessageBox::Ok);
    return;
  }
  if (!book_.removeAccount(Account(names_.at(0), names_.at(1), names_.at(2)))) {
    QMessageBox::warning(this, "Cannot delete!", "The account still have transactions link to it!", QMessageBox::Ok);
    return;
  }
  QTreeWidgetItem* item = tree_widget_->currentItem();
  QTreeWidgetItem* parent = item->parent();
  delete item;
  if (parent->childCount() == 0) {
    delete parent;
  }
}
