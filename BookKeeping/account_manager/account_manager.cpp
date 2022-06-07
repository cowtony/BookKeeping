#include "account_manager.h"
#include "ui_account_manager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QDropEvent>

#include "book.h"

AccountManager::AccountManager(Book& book, QWidget *parent)
    : QMainWindow(parent), ui_(new Ui::AccountManager), book_(book), account_model_(book) {
  ui_->setupUi(this);
//  tree_widget_ = new TreeWidget(book_);
//  ui_->gridLayout->addWidget(tree_widget_, 0, 0);
  tree_view_ = std::make_unique<QTreeView>();
  tree_view_->setModel(&account_model_);
  ui_->gridLayout->addWidget(tree_view_.get(), 0, 0, 1, 2);

  account_model_.setupAccounts(book_.queryAllAccountsFrom({Account::Asset, Account::Liability, Account::Revenue, Account::Expense}));
  tree_view_->setDragDropMode(QAbstractItemView::InternalMove);
  tree_view_->setSelectionMode(QAbstractItemView::SingleSelection);
  tree_view_->setDropIndicatorShown(true);
  tree_view_->setColumnWidth(0, 200);

  connect(&account_model_, &AccountsModel::errorMessage, this, &AccountManager::onReceiveErrorMessage);
  connect(tree_view_->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountManager::onCurrentItemChanged);
}

void AccountManager::onCurrentItemChanged(const QModelIndex& current, const QModelIndex& /* previous */) {
  ui_->pushButton_Add   ->setEnabled(AccountsModel::getItem(current)->depth() != 3);  // Can add under type or category.
  ui_->pushButton_Delete->setEnabled(AccountsModel::getItem(current)->depth() == 3);  // Can delete meta account.
}

void AccountManager::onReceiveErrorMessage(const QString& message) {
  QMessageBox::warning(this, "WARNING", message, QMessageBox::Ok);
}

void AccountManager::on_pushButton_Add_clicked() {
  AccountTreeNode* item = AccountsModel::getItem(tree_view_->currentIndex());
  switch (item->depth()) {
    case 1: {  // Account type level
      bool ok;
      QString category_name = QInputDialog::getText(this, "Add category into table: " + item->name(), "Category:", QLineEdit::Normal, "", &ok);
      if (ok and !category_name.isEmpty()) {
        if (book_.insertCategory(item->accountType(), category_name)) {
          QModelIndex category_index = account_model_.appendRow(tree_view_->currentIndex(), category_name);
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
          account_model_.appendRow(tree_view_->currentIndex(), account_name);
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
  AccountTreeNode* item = AccountsModel::getItem(tree_view_->currentIndex());
  if (item->depth() != 3) {
    QMessageBox::warning(this, "Warning", "Cannot delete the whole type or category.", QMessageBox::Ok);
    return;
  }
  if (!book_.removeAccount(*item->account())) {
    QMessageBox::warning(this, "Cannot delete!", "The account still have transactions link to it!", QMessageBox::Ok);
    return;
  }
  account_model_.removeItem(tree_view_->currentIndex());
}

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
