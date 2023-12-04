#include "accounts_model.h"

#include <QApplication>
#include <QMessageBox>

#include "home_window/home_window.h"

AccountsModel::AccountsModel(QObject *parent):
  QAbstractItemModel(parent),
  book_(static_cast<HomeWindow*>(parent)->book),
  user_id_(static_cast<HomeWindow*>(parent)->user_id) {
    root_ = new AccountTreeNode();
}

AccountsModel::~AccountsModel() {
    delete root_;
}

void AccountsModel::setupCategoriesAndAccounts(const QList<QSharedPointer<Account>>& categories, const QList<QSharedPointer<Account>>& accounts) {
    root_->clear();
    // Preset 4 basic account type in case there is no account for one of them.
    for (const char* const& type_name : {"Asset", "Liability", "Revenue", "Expense"}) {
        root_->insertChild(new AccountTreeNode(type_name));
    }
    // Setup Categories.
    for (const QSharedPointer<Account>& category : categories) {
        AccountTreeNode* node = root_->childAt(category->typeName());
        Q_ASSERT(node);
        Q_ASSERT(!node->childAt(category->categoryName()));
        node->insertChild(new AccountTreeNode(category->categoryName()));
        node = node->childAt(category->categoryName());
        // Set additional data.
        Q_ASSERT(node->depth() == 2);
        node->setAccount(category);
    }
    // Setup Accounts.
    for (const QSharedPointer<Account>& account : accounts) {
        AccountTreeNode* node = root_->childAt(account->typeName());
        Q_ASSERT(node);
        node = node->childAt(account->categoryName());
        Q_ASSERT(node);
        Q_ASSERT(!node->childAt(account->accountName()));
        node->insertChild(new AccountTreeNode(account->accountName()));
        node = node->childAt(account->accountName());
        // Set additional data.
        Q_ASSERT(node->depth() == 3);
        node->setAccount(account);
    }
    emit dataChanged(QModelIndex(), QModelIndex());
}

QVariant AccountsModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0: return "Account Name";
        case 1: return "Comment";
        case 2: return "Is Investment?";
        default: return QVariant();
      }
    }
  }
  return QVariant();
}

//bool AccountModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
//{
//  if (value != headerData(section, orientation, role)) {
//    // FIXME: Implement me!
//    emit headerDataChanged(orientation, section, section);
//    return true;
//  }
//  return false;
//}

QModelIndex AccountsModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }
  AccountTreeNode* parent_node;
  if (!parent.isValid()) {
    parent_node = root_;
  } else {
    parent_node = getItem(parent);
  }
  if (AccountTreeNode* node = parent_node->childAt(row)) {
    return createIndex(row, column, node);
  } else {
    return QModelIndex();
  }
}

QModelIndex AccountsModel::parent(const QModelIndex &index) const {
  if (!index.isValid()) {
    return QModelIndex();  // Root won't have parent.
  }

  AccountTreeNode* node = getItem(index);
  AccountTreeNode* parent_node = node->parent();
  if (parent_node == root_) {
    return QModelIndex();  // Root won't have ModelIndex.
  }
  return createIndex(parent_node->index(), 0, parent_node);
}

int AccountsModel::rowCount(const QModelIndex& parent) const {
  if (!parent.isValid()) {
    return root_->childCount();
  }
  AccountTreeNode* parent_node = getItem(parent);
  return parent_node->childCount();
}

int AccountsModel::columnCount(const QModelIndex& /* parent */) const {
  return 3;
}

//bool AccountModel::hasChildren(const QModelIndex &parent) const
//{
//  // FIXME: Implement me!
//}

//bool AccountModel::canFetchMore(const QModelIndex &parent) const
//{
//  // FIXME: Implement me!
//  return false;
//}

//void AccountModel::fetchMore(const QModelIndex &parent)
//{
//  // FIXME: Implement me!
//}

QVariant AccountsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    AccountTreeNode* node = getItem(index);
    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
                case 0:  // Name column
                    return node->name();
                case 1:  // Comment column
                    if (node->depth() == 3) {
                        return node->account()->comment();
                    } else {
                        return "";
                    }
                default: break;
            }
            break;
        case Qt::FontRole:
            switch (node->depth()) {
                case 1: return kTypeFont;
                case 2: return kCategoryFont;
                case 3: return kAccountFont;
                default: break;
            }
            break;
        case Qt::CheckStateRole:
            if (index.column() == 2 && node->depth() == 3 && node->accountType() == Account::Asset) {
                return node->account()->isInvestment()? Qt::Checked : Qt::Unchecked;
            }
            break;
        default: break;
    }
    return QVariant();
}

// TODO: Add default text when double clicked.
bool AccountsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (data(index, role) == value) {
        return false;
    }
    AccountTreeNode* node = getItem(index);
    switch (role) {
        case Qt::EditRole:
            switch (node->depth()) {
                case 1:  // Account Type
                    return false;
                case 2: { // Account Category
                    if (value.toString().isEmpty()) {
                        return false;
                    }
                    if (book_.getCategory(user_id_, node->accountType(), value.toString())) {
                        emit errorMessage("Group name " + value.toString() + " already exist.");
                        return false;
                    }
                    QApplication::setOverrideCursor(Qt::WaitCursor);
                    bool success = book_.renameCategory(user_id_, node->accountType(), node->accountGroup(), value.toString());
                    QApplication::restoreOverrideCursor();
                    if (success) {
                        node->setName(value.toString());
                        // TODO: emit Category name Changed
                        // TODO: Currently the display will not refresh.
                    } else {
                        return false;
                    }
                    break;
                }
                case 3:  // Account
                    switch (index.column()) {
                        case 0: {  // Account Name column
                            QString new_account_name = value.toString();
                            if (new_account_name.isEmpty()) {
                                return false;
                            }
                            auto old_account = node->account();
                            if (book_.getAccount(user_id_, old_account->accountType(), old_account->categoryName(), new_account_name) != nullptr) {
                                // new_account_name exist, perform merge.
                                if (!node->account()->comment().isEmpty()) {
                                    emit errorMessage("The account to be merged still have unempty comment.");
                                    return false;
                                }
                                QMessageBox message_box;
                                message_box.setText("Do you want to merge with existing account?");
                                message_box.setInformativeText("OldName: " + old_account->accountName() + ", NewName: " + new_account_name);
                                message_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
                                message_box.setDefaultButton(QMessageBox::Ok);
                                switch (message_box.exec()) {
                                    case QMessageBox::Discard: return false;
                                }
                                QApplication::setOverrideCursor(Qt::WaitCursor);
                                // TODO: Add merge account feature: book_.mergeAccount();
                                QApplication::restoreOverrideCursor();
                                emit errorMessage("The merge account feature has not been implemented.");
                                return false;
                                removeItem(index);
                            } else {
                                // Account not exist, perform rename.
                                QString error_msg = book_.renameAccount(user_id_, *old_account, new_account_name);
                                if (!error_msg.isEmpty()) {
                                    emit errorMessage(error_msg);
                                    return false;
                                }
                                if (!node->setName(new_account_name)) {
                                    return false;
                                }
                            }
                            // TODO: emit account name Changed
                            break;
                        }
                        case 1: { // Comment column
                            if(!book_.updateAccountComment(node->account()->accountId(), value.toString())) {
                                return false;
                            }
                            node->account()->setComment(value.toString());
                            break;
                        }
                        default: return false;
                    }
                    break;
                default:
                    return false;
            }
            break;
        case Qt::CheckStateRole:
            if (index.column() == 2 && node->depth() == 3 && node->accountType() == Account::Asset) {
                // TODO: Connect to database.
                QString error = book_.setInvestment(user_id_, *dynamic_cast<AssetAccount*>(node->account().get()), value.toBool());
                if (!error.isEmpty()) {
                    emit errorMessage(error);
                    return false;
                }
                node->account()->setIsInvestment(value.toBool());
                // TODO: Also add/remove the item to Revenue::Investment
            }
            break;
        default:
            return false;
    }

    emit dataChanged(index, index, {role});
    return true;
}

Qt::ItemFlags AccountsModel::flags(const QModelIndex& index) const {
    auto node = getItem(index);
    if (!index.isValid()) {
        return QAbstractItemModel::flags(index);
    }
    if (node->accountType() == Account::Revenue && node->accountGroup() == "Investment") {
        return QAbstractItemModel::flags(index) & ~Qt::ItemIsEnabled;
    }
    switch (node->depth()) {
        case 1:  // Account type
            return (Qt::ItemIsEnabled | Qt::ItemIsSelectable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled;
        case 2:  // Account group
            return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDropEnabled) & ~Qt::ItemIsDragEnabled;
        case 3:  // Account name
            if (node->accountType() == Account::Asset and index.column() == 2) {
                return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
            } else {
                return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled;
            }
    }
    return Qt::NoItemFlags;
}

QStringList AccountsModel::mimeTypes() const {
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

QMimeData* AccountsModel::mimeData(const QModelIndexList& indexes) const {
  QMimeData *mimeData = new QMimeData;
  QByteArray encodedData;

  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  for (const QModelIndex &index : indexes) {
    if (index.isValid()) {
      QString text = data(index, Qt::DisplayRole).toString();
      stream << text;
    }
  }

  mimeData->setData("application/vnd.text.list", encodedData);
  return mimeData;
}

bool AccountsModel::canDropMimeData(const QMimeData* data, Qt::DropAction /* action */, int /* row */, int column, const QModelIndex& /* parent */) const {
  if (!data->hasFormat("application/vnd.text.list")) {
    return false;
  }

  if (column > 0) {
    return false;
  }

  return true;
}

bool AccountsModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
  if (!canDropMimeData(data, action, row, column, parent)) {
    return false;
  }

  if (action == Qt::IgnoreAction) {
    return true;
  }

  int beginRow;
  if (row != -1) {
    beginRow = row;
  } else if (parent.isValid()) {
    beginRow = parent.row();
  } else {
    beginRow = rowCount(QModelIndex());
  }

  QByteArray encodedData = data->data("application/vnd.text.list");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  QStringList newItems;
  int rows = 0;
  while (!stream.atEnd()) {
    QString text;
    stream >> text;
    newItems << text;
    ++rows;
  }

  insertRows(beginRow, rows, parent);
  for (const QString &text : qAsConst(newItems)) {
    QModelIndex idx = index(beginRow, 0, parent);
    setData(idx, text, Qt::DisplayRole);
    beginRow++;
  }
  return true;
}

Qt::DropActions AccountsModel::supportedDragActions() const {
  return Qt::MoveAction;
}

Qt::DropActions AccountsModel::supportedDropActions() const {
  return Qt::MoveAction;
}

bool AccountsModel::insertRows(int row, int count, const QModelIndex& parent) {
    AccountTreeNode* parent_node = getItem(parent);
    if (!parent_node) {
        return false;
    }
    beginInsertRows(parent, row, row + count - 1);
    for (int r = row; r < row + count; r++) {
        // TODO: change the default value (which is datatime now.)
        parent_node->insertChild(new AccountTreeNode(QDateTime::currentDateTime().toString()), r);
    }
    endInsertRows();
    return true;
}

QModelIndex AccountsModel::appendRow(const QModelIndex& parent, const QString& name, QSharedPointer<Account> account) {
    AccountTreeNode* parent_node = getItem(parent);
    if (!parent_node) {
        return QModelIndex();
    }
    beginInsertRows(parent, parent_node->childCount(), parent_node->childCount());
    bool success = parent_node->insertChild(new AccountTreeNode(name));
    endInsertRows();
    if (!success) {
        return QModelIndex();
    }
    parent_node->childAt(name)->setAccount(account);
    return index(parent_node->childCount() - 1, 0, parent);
}

//bool AccountModel::insertColumns(int column, int count, const QModelIndex &parent)
//{
//  beginInsertColumns(parent, column, column + count - 1);
//  // FIXME: Implement me!
//  endInsertColumns();
//}

bool AccountsModel::removeRows(int row, int count, const QModelIndex& parent) {
    beginRemoveRows(parent, row, row + count - 1);
    AccountTreeNode* parent_node = getItem(parent);
    parent_node->removeChildren(row, count);
    endRemoveRows();
    return true;
}

bool AccountsModel::removeItem(const QModelIndex& index) {
  return removeRows(index.row(), 1, index.parent());
}

//bool AccountModel::removeColumns(int column, int count, const QModelIndex &parent)
//{
//  beginRemoveColumns(parent, column, column + count - 1);
//  // FIXME: Implement me!
//  endRemoveColumns();
//}

// static
AccountTreeNode* AccountsModel::getItem(const QModelIndex& index) {
    return static_cast<AccountTreeNode*>(index.internalPointer());
}
