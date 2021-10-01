#include "book_model.h"

BookModel::BookModel(QObject *parent)
  : QAbstractTableModel(parent) {}

QVariant BookModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    switch (orientation) {
    case Qt::Horizontal:
      if (section < static_cast<int>(kColumnNames.size())) {
        return kColumnNames.at(section);
      }
    case Qt::Vertical:
      switch (section) {
      case 0:
        return "From:";
      case 1:
        return "To:";
      default:
        return section - kReservedFilterRow + 1;
      }
    }
  }
  return QVariant();
}

//bool BookModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
//{
//  if (value != headerData(section, orientation, role)) {
//    // FIXME: Implement me!
//    emit headerDataChanged(orientation, section, section);
//    return true;
//  }
//  return false;
//}

int BookModel::rowCount(const QModelIndex &parent) const {
  return kReservedFilterRow + transactions_.size();
}

int BookModel::columnCount(const QModelIndex &parent) const {
  return kColumnNames.size();
}

//bool BookModel::hasChildren(const QModelIndex &parent) const
//{
//  // FIXME: Implement me!
//}

//bool BookModel::canFetchMore(const QModelIndex &parent) const
//{
//  // FIXME: Implement me!
//  return false;
//}

//void BookModel::fetchMore(const QModelIndex &parent)
//{
//  // FIXME: Implement me!
//}

QVariant BookModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    Q_ASSERT(index.row() < kReservedFilterRow + transactions_.size());
    if (index.row() < kReservedFilterRow) {
      return QVariant();
    }
    const auto& transaction = transactions_.at(index.row() - kReservedFilterRow);
    switch (index.column()) {
    case 0:
      return transaction.date_time_.toString(DATE_TIME_FORMAT);
    case 1:
      return transaction.description_;
    case 2:
    case 3:
    case 4:
    case 5:
      return transaction.dataToString(kTableList.at(index.column() - 2)).replace("; ", "\n");
    }
  }
  return QVariant();
}

void BookModel::SetTransactions(const QList<Transaction>& transactions) {
  transactions_ = transactions.mid(0, kMaximumTransactions);
  QModelIndex top_left = createIndex(2,0);
  QModelIndex bottom_right = createIndex(kReservedFilterRow + transactions_.size() - 1, columnCount() - 1);
  emit dataChanged(top_left, bottom_right, {Qt::DisplayRole});
  emit layoutChanged();  // To update the rows, not exactly sure how this works.
}

//bool BookModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//  if (data(index, role) != value) {
//    // FIXME: Implement me!
//    emit dataChanged(index, index, QVector<int>() << role);
//    return true;
//  }
//  return false;
//}

//Qt::ItemFlags BookModel::flags(const QModelIndex &index) const
//{
//  if (!index.isValid())
//    return Qt::NoItemFlags;

//  return Qt::ItemIsEditable; // FIXME: Implement me!
//}

//bool BookModel::insertRows(int row, int count, const QModelIndex &parent) {
//  beginInsertRows(parent, row, row + count - 1);
//  // FIXME: Implement me!
//  endInsertRows();
//}

//bool BookModel::insertColumns(int column, int count, const QModelIndex &parent)
//{
//  beginInsertColumns(parent, column, column + count - 1);
//  // FIXME: Implement me!
//  endInsertColumns();
//}

//bool BookModel::removeRows(int row, int count, const QModelIndex &parent) {
//  beginRemoveRows(parent, row, row + count - 1);
//  // FIXME: Implement me!
//  endRemoveRows();
//}

//bool BookModel::removeColumns(int column, int count, const QModelIndex &parent)
//{
//  beginRemoveColumns(parent, column, column + count - 1);
//  // FIXME: Implement me!
//  endRemoveColumns();
//}
