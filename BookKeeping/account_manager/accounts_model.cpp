#include "accounts_model.h"

#include <QApplication>
#include <QMessageBox>

AccountsModel::AccountsModel(Book& book, QObject *parent)
  : QAbstractItemModel(parent), book_(book) {
  root_ = new AccountTreeNode();
}

AccountsModel::~AccountsModel() {
  delete root_;
}

void AccountsModel::setupAccounts(const QList<Account>& accounts) {
  root_->clear();
  for (const Account& account : accounts) {
    AccountTreeNode* current_node = root_;
    for (const QString& name : {account.typeName(), account.category, account.name}) {
      if (current_node->childAt(name) == nullptr) {
        current_node->insertChild(new AccountTreeNode(name));
      }
      current_node = current_node->childAt(name);
      if (current_node->depth() == 3) {
        current_node->setComment(account.comment);
      }
    }
  }
  emit dataChanged(QModelIndex(), QModelIndex());
}

QVariant AccountsModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0: return "Account Name";
        case 1: return "Comment";
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
    parent_node = static_cast<AccountTreeNode*>(parent.internalPointer());
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

  AccountTreeNode* node = static_cast<AccountTreeNode*>(index.internalPointer());
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
  AccountTreeNode* parent_node = static_cast<AccountTreeNode*>(parent.internalPointer());
  return parent_node->childCount();
}

int AccountsModel::columnCount(const QModelIndex& /* parent */) const {
  return 2;
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
    case Qt::EditRole:
      switch (index.column()) {
        case 0: return node->name();
        case 1: return node->comment();
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
    default: break;
  }
  return QVariant();
}

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
        case 2: { // Account Group
          if (value.toString().isEmpty()) {
            return false;
          }
          if (book_.categoryExist(node->accountType(), value.toString())) {
            QMessageBox message_box;
            message_box.setText("Group name " + value.toString() + " already exist.");
            message_box.exec();
            return false;
          }
          QApplication::setOverrideCursor(Qt::WaitCursor);
          bool success = book_.renameCategory(node->accountType(), node->accountGroup(), value.toString());
          QApplication::restoreOverrideCursor();
          if (success) {
            node->setName(value.toString());
          } else {
            return false;
          }
          break;
        }
        case 3:  // Account
          switch (index.column()) {
            case 0: {  // Account Name
              if (value.toString().isEmpty()) {
                return false;
              }
              Account old_account = node->account();
              Account new_account = old_account;
              new_account.name = value.toString();
              bool account_exist = book_.accountExist(new_account);

              if (account_exist) {  // new_account_name exist, perform merge.
                if (!node->comment().isEmpty()) {
                  QMessageBox message_box;
                  message_box.setText("The account to be merged still have unempty comment.");
                  message_box.exec();
                  return false;
                }
                QMessageBox message_box;
                message_box.setText("Do you want to merge with existing account?");
                message_box.setInformativeText("OldName: " + old_account.name + ", NewName: " + value.toString());
                message_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
                message_box.setDefaultButton(QMessageBox::Ok);
                switch (message_box.exec()) {
                  case QMessageBox::Discard:
                    return false;
                }
              }

              QApplication::setOverrideCursor(Qt::WaitCursor);
              bool success = book_.moveAccount(old_account, new_account);
              QApplication::restoreOverrideCursor();
              if (!success) {
                return false;
              }
              if (account_exist) {
                removeItem(index);
              } else { // Simply rename.
                if (!node->setName(value.toString())) {
                  return false;
                }
              }

              break;
            }
            case 1: { // Comment
              if(!book_.updateAccountComment(node->account(), value.toString())) {
                return false;
              }
              node->setComment(value.toString());
              break;
            }
            default: return false;
          }
        default:
          return false;
      }
      break;
    default:
      return false;
  }

  emit dataChanged(index, index, {role});
  return true;
}

Qt::ItemFlags AccountsModel::flags(const QModelIndex& index) const {
  AccountTreeNode* node = getItem(index);
  if (!index.isValid() or !node) {
    return QAbstractItemModel::flags(index);
  }
  switch (node->depth()) {
    case 1:  // Account type
      return (Qt::ItemIsEnabled | Qt::ItemIsSelectable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled;
    case 2:  // Account group
      return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDropEnabled) & ~Qt::ItemIsDragEnabled;
    case 3:  // Account name
      return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled;
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
  beginInsertColumns(parent, row, row + count - 1);
  for (int r = row; r < row + count; r++) {
    parent_node->insertChild(new AccountTreeNode(QDateTime::currentDateTime().toString()), r);
  }
  endInsertColumns();
  return true;
}

QModelIndex AccountsModel::appendRow(const QModelIndex& parent, const QString& name) {
  AccountTreeNode* parent_node = getItem(parent);
  if (!parent_node) {
    return QModelIndex();
  }
  beginInsertRows(parent, parent_node->childCount(), parent_node->childCount());
  bool success = parent_node->insertChild(new AccountTreeNode(name));
  endInsertRows();
  if (!success) {
    return QModelIndex();
  } else {
    return QModelIndex();
  }
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

AccountTreeNode* AccountsModel::getItem(const QModelIndex& index) const {
  return static_cast<AccountTreeNode*>(index.internalPointer());
}
