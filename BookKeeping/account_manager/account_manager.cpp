#include "account_manager.h"
#include "ui_account_manager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QDropEvent>

#include "book.h"

TreeWidget::TreeWidget(Book& book, QWidget *parent)
    : QTreeWidget(parent), book_(book) {
  setSelectionMode(QAbstractItemView::SingleSelection);
  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);
  setDragDropMode(QAbstractItemView::InternalMove);

  headerItem()->setText(0, "Account Name");
  headerItem()->setText(1, "Comment");
  setColumnWidth(0, 150);
}

void TreeWidget::startDrag(Qt::DropActions actions) {
  QTreeWidget::startDrag(actions);  // TODO: This will cause "program has unexpectedly finished", didn't found out why.
}

void TreeWidget::dragEnterEvent(QDragEnterEvent *event) {
  drag_from_.clear();
  QTreeWidgetItem *item = currentItem();
  while (item != nullptr) {
    drag_from_.push_front(item->text(0));
    qDebug() << item->text(0);
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

void TreeWidget::onItemChanged(QTreeWidgetItem* item, int column) {
  // TODO: implement this function.
  qDebug() << item->text(column);

}

AccountManager::AccountManager(Book& book, QWidget *parent)
    : QMainWindow(parent), ui_(new Ui::AccountManager), book_(book), account_model_(book) {
  ui_->setupUi(this);
  tree_widget_ = new TreeWidget(book_);
  tree_view_ = new QTreeView;
  tree_view_->setModel(&account_model_);

  ui_->gridLayout->addWidget(tree_widget_, 0, 0, 1, 2);
  ui_->gridLayout->addWidget(tree_view_, 0, 2, 1, 1);
  connect(tree_widget_, &QTreeWidget::currentItemChanged, this, &AccountManager::onCurrentItemChanged);

  for (const Account::Type& account_type : {Account::Asset, Account::Liability, Account::Expense, Account::Revenue}) {
    QTreeWidgetItem* account_type_item = AddAccountType(account_type);
    for (const QString& category : book_.queryCategories(account_type)) {
      QTreeWidgetItem* account_category_item = AddAccountGroup(account_type_item, category);
      for (const Account& account : book_.queryAccounts(account_type, category)) {
        AddAccount(account_category_item, account.name, account.comment);
      }
    }
  }
  // Connect after accounts are populated, since adding new child also deemed as itemChange.
  connect(tree_widget_, &QTreeWidget::itemChanged, tree_widget_, &TreeWidget::onItemChanged);

  // WIP: TreeView
  account_model_.setupAccounts(book_.queryAllAccountsFrom({Account::Asset, Account::Liability, Account::Revenue, Account::Expense}));
  tree_view_->setDragDropMode(QAbstractItemView::InternalMove);
  tree_view_->setSelectionMode(QAbstractItemView::SingleSelection);
  tree_view_->setDropIndicatorShown(true);
  tree_view_->setColumnWidth(0, 300);
}

AccountManager::~AccountManager() {
//  delete ui;
}

void AccountManager::onCurrentItemChanged(QTreeWidgetItem *current) {
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
          QTreeWidgetItem* account_category_item = AddAccountGroup(tree_widget_->currentItem(), category_name);
          AddAccount(account_category_item, category_name); // Insert the dummy account name item with the same name.
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
          AddAccount(tree_widget_->currentItem(), account_name);
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
            onCurrentItemChanged(tree_widget_->currentItem());
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

QTreeWidgetItem* AccountManager::AddAccountType(Account::Type account_type) {
  QTreeWidgetItem* account_type_item = new QTreeWidgetItem(tree_widget_);
  account_type_item->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
  account_type_item->setText(0, Account::kTableName.value(account_type));
  account_type_item->setFont(0, kTypeFont);
  return account_type_item;
}

QTreeWidgetItem* AccountManager::AddAccountGroup(QTreeWidgetItem* category, const QString& group_name) {
  QTreeWidgetItem* account_category_item = new QTreeWidgetItem(category);
  account_category_item->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled) & ~Qt::ItemIsDragEnabled);
  account_category_item->setText(0, group_name);
  account_category_item->setFont(0, kCategoryFont);
  return account_category_item;
}

QTreeWidgetItem* AccountManager::AddAccount(QTreeWidgetItem* category, const QString& account_name, const QString& comment) {
  QTreeWidgetItem* account_item = new QTreeWidgetItem(category);
  // TODO: enable after fix the drag drop issue.
  account_item->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable /*| Qt::ItemIsDragEnabled*/) & ~Qt::ItemIsDropEnabled);
  account_item->setText(0, account_name);
  account_item->setText(1, comment);
  account_item->setFont(0, kAccountFont);
  return account_item;
}
