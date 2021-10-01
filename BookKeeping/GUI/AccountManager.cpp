#include "AccountManager.h"
#include "ui_AccountManager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QDropEvent>

#include "Book.h"

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
  QModelIndex index = indexAt(event->pos());
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
  ui_->gridLayout->addWidget(tree_widget_, 0, 0, 1, 3);
  connect(tree_widget_, &QTreeWidget::currentItemChanged, this, &AccountManager::onTreeWidgetItemChanged);
  for (const Account::TableType &tableType : {Account::Asset, Account::Liability, Account::Expense, Account::Revenue}) {
    QTreeWidgetItem* accountTypeItem = new QTreeWidgetItem(tree_widget_);
    accountTypeItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
    accountTypeItem->setText(0, Account::kTableName.value(tableType));
    accountTypeItem->setFont(0, kTableFont);

    for (const QString &category : book_.getCategories(tableType)) {
      QTreeWidgetItem* accountCategoryItem = new QTreeWidgetItem(accountTypeItem);
      accountCategoryItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled) & ~Qt::ItemIsDragEnabled);
      accountCategoryItem->setText(0, category);
      accountCategoryItem->setFont(0, kCategoryFont);

      for (const QString &name : book_.getAccountNames(tableType, category)) {
        QTreeWidgetItem* accountItem = new QTreeWidgetItem(accountCategoryItem);
        accountItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
        accountItem->setText(0, name);
        accountItem->setFont(0, kAccountFont);
      }
    }
  }
}

AccountManager::~AccountManager() {
//    delete ui;
}

void AccountManager::onTreeWidgetItemChanged(QTreeWidgetItem *current) {
  QTreeWidgetItem* node = current;

  names_.clear();
  while (node != nullptr) {
    names_.push_front(node->text(0));
    node = node->parent();
  }
  switch (names_.size()) {
    case 1:  // Table
      ui_->pushButton_Add   ->setEnabled(true);
      ui_->pushButton_Rename->setEnabled(false);
      ui_->pushButton_Delete->setEnabled(false);
      break;
    case 2:  // Category
      ui_->pushButton_Add   ->setEnabled(true);
      ui_->pushButton_Rename->setEnabled(true);
      ui_->pushButton_Delete->setEnabled(true);
      break;
    case 3:  // Name
      ui_->pushButton_Add   ->setEnabled(false);
      ui_->pushButton_Rename->setEnabled(true);
      ui_->pushButton_Delete->setEnabled(true);
      break;
  }
}

void AccountManager::on_pushButton_Add_clicked() {
  switch (names_.size()) {
    case 1: { // Table Level
      bool ok;
      QString category = QInputDialog::getText(this, "Add category into table " + names_.at(0), "Category:", QLineEdit::Normal, "", &ok);
      if (ok)
      {
          if (book_.insertCategory(names_.at(0), category))
          {
              QTreeWidgetItem* categoryItem = new QTreeWidgetItem(tree_widget_->currentItem());
              categoryItem->setText(0, category);
              categoryItem->setFont(0, kCategoryFont);
              for (const QString &accountName : book_.getAccountNames(Account::kTableName.key(names_.at(0)), category))
              {
                  QTreeWidgetItem* accountItem = new QTreeWidgetItem(categoryItem);
                  accountItem->setText(0, accountName);
                  accountItem->setFont(0, kAccountFont);
              }
          }
      }
      break;
    }
    case 2: { // Category Level
      bool ok;
      QString accountName = QInputDialog::getText(this, "Add Account into category " + names_[1], "Name:", QLineEdit::Normal, "", &ok);
      if (ok && !accountName.isEmpty())
      {
          if (book_.insertAccount(Account(names_.at(0), names_.at(1), accountName)))
          {
              QTreeWidgetItem* l_item = new QTreeWidgetItem;
              tree_widget_->currentItem()->addChild(l_item);
              l_item->setText(0, accountName);
              l_item->setFont(0, kAccountFont);
          }
          else {
              qDebug() << Q_FUNC_INFO << "Insert failed" << names_.at(0) << names_.at(1) << accountName;
          }
      }
      else {
          qDebug() << Q_FUNC_INFO << ok << accountName;
      }
      break;
    }
    case 3:  // Account Level
      break;
  }
}

void AccountManager::on_pushButton_Rename_clicked() {
  switch (names_.size()) {
    case 1:  // Table Level
      break;
    case 2: { // Category Level
      bool ok;
      QString newName = QInputDialog::getText(this, "Rename Category", "Name:", QLineEdit::Normal, names_.at(1), &ok);
      if (ok && !newName.isEmpty()) {
        if (book_.categoryExist(names_.at(0), newName)) {
          QMessageBox messageBox;
          messageBox.setText("Error: The category already exist.");
          messageBox.exec();
        } else {
          QApplication::setOverrideCursor(Qt::WaitCursor);
          if (book_.renameCategory(names_.at(0), names_.at(1), newName)) {
            tree_widget_->currentItem()->setText(0, newName);
            names_[1] = newName;
            emit categoryChanged();
          }
          QApplication::restoreOverrideCursor();
        }
      }
      break;
    }
    case 3: { // Account Level
      bool ok;
      QString newName = QInputDialog::getText(this, "Rename Account", "Name:", QLineEdit::Normal, names_.at(2), &ok);
      if (ok && !newName.isEmpty()) {
        if (book_.accountExist(Account(names_.at(0), names_.at(1), newName))) {
            QMessageBox messageBox;
            messageBox.setText("Do you want to merge with existing account?");
            messageBox.setInformativeText("OldName: " + names_[2] + ", NewName: " + newName);
            messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            messageBox.setDefaultButton(QMessageBox::Ok);
            switch (messageBox.exec()) {
              case QMessageBox::Ok:
                QApplication::setOverrideCursor(Qt::WaitCursor);
                if (book_.moveAccount(Account(names_.at(0), names_.at(1), names_.at(2)),
                                      Account(names_.at(0), names_.at(1), newName))) {
                  delete tree_widget_->currentItem();
                  onTreeWidgetItemChanged(tree_widget_->currentItem());
                  emit accountNameChanged();
                }
                QApplication::restoreOverrideCursor();
                return;
              case QMessageBox::Discard:
                return;
            }
        } else if (book_.moveAccount(Account(names_.at(0), names_.at(1), names_.at(2)),
                                     Account(names_.at(0), names_.at(1), newName))) {
          tree_widget_->currentItem()->setText(0, newName);
          names_[2] = newName;
          emit accountNameChanged();
        }
      }
      break;
    }
  }
}

void AccountManager::on_pushButton_Delete_clicked() {
    switch (names_.size()) {
      case 1:  // Table Level
        break;
      case 2:  // Category Level
        if (book_.removeCategory(names_.at(0), names_.at(1))) {
          delete tree_widget_->currentItem();
        } else {
          QMessageBox messageBox;
          messageBox.setText("Cannot delete!\nThe category still have account in it!");
          messageBox.exec();
        }
        break;
      case 3:  // Account Level
        if (book_.removeAccount(Account(names_.at(0), names_.at(1), names_.at(2)))) {
          delete tree_widget_->currentItem();
        } else {
          QMessageBox messageBox;
          messageBox.setText("Cannot delete!\nThe account still have transactions link to it!");
          messageBox.exec();
        }
        break;
    }
}


