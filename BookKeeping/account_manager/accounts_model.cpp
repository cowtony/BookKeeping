#include "accounts_model.h"

AccountsModel::AccountsModel(QObject *parent)
  : QAbstractItemModel(parent) {
  root_node_ = new AccountTreeNode();
}

AccountsModel::~AccountsModel() {
  delete root_node_;
}

void AccountsModel::setupAccounts(const QList<Account>& accounts) {
  for (const Account& account : accounts) {
    AccountTreeNode* current_node = root_node_;
    for (const QString& name : {account.typeName(), account.category, account.name}) {
      if (current_node->childAt(name) == nullptr) {
        current_node->insertChild(new AccountTreeNode(name));
      }
      current_node = current_node->childAt(name);
    }
  }
  emit dataChanged(QModelIndex(), QModelIndex());
}

QVariant AccountsModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
        case 0:  return "Account Name";
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
    parent_node = root_node_;
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
  if (parent_node == root_node_) {
    return QModelIndex();  // Root won't have ModelIndex.
  }
  return createIndex(parent_node->index(), 0, parent_node);
}

int AccountsModel::rowCount(const QModelIndex& parent) const {
  if (!parent.isValid()) {
    return root_node_->childCount();
  }
  AccountTreeNode* parent_node = static_cast<AccountTreeNode*>(parent.internalPointer());
  return parent_node->childCount();
}

int AccountsModel::columnCount(const QModelIndex &parent) const {
  return 1;
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
  AccountTreeNode* node = static_cast<AccountTreeNode*>(index.internalPointer());
  switch (role) {
  case Qt::DisplayRole:
    return node->name();
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
  if (role == Qt::DisplayRole) {
    AccountTreeNode* node = getItem(index);
    if (!node->setName(value.toString())) {
      return false;
    }
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
    case 1:
      return (Qt::ItemIsEnabled | Qt::ItemIsSelectable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled;
    case 2:
      return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled) & ~Qt::ItemIsDragEnabled;
    case 3:
      return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled;
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

bool AccountsModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const {
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(parent);

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
