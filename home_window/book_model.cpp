#include "book_model.h"

BookModel::BookModel(QObject *parent): QAbstractTableModel(parent) {}

void BookModel::SetTransactions(const QList<Transaction>& transactions) {
    transactions_ = transactions.mid(0, kMaximumTransactions);
    sum_transaction_.clear();
    for (const Transaction& transaction : transactions) {
        sum_transaction_ += transaction;
    }
    sum_transaction_.date_time = QDateTime();
    sum_transaction_.description = "SUM:";
    QModelIndex top_left = createIndex(2, 0);
    QModelIndex bottom_right = createIndex(kReservedFilterRow + transactions_.size(), columnCount() - 1);
    emit dataChanged(top_left, bottom_right, {Qt::DisplayRole});
    emit layoutChanged();  // To update the rows, not exactly sure how this works.
}

QVariant BookModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        switch (orientation) {
            case Qt::Horizontal:
                if (section < static_cast<int>(kColumnNames.size())) {
                    return kColumnNames.at(section);
                }
                break;
            case Qt::Vertical:
                switch (section) {
                    case 0:  return "From:";
                    case 1:  return "To:";
                    default: return section - kReservedFilterRow + 1;
                }
        }
    }
    return QVariant();
}

int BookModel::rowCount(const QModelIndex& /* parent */) const {
    return kReservedFilterRow + transactions_.size() + /* sum_transaction */ 1;
}

int BookModel::columnCount(const QModelIndex& /* parent */) const {
    return kColumnNames.size();
}

QVariant BookModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
        case Qt::FontRole:
            // Bold the sum row.
            if (index.row() == kReservedFilterRow + transactions_.size()) {
                QFont font;
                font.setBold(true);
                return font;
            }
            break;
        case Qt::BackgroundRole: {
            // Descriptions need pay attention.
            if (index.row() - kReservedFilterRow >= 0 and index.row() - kReservedFilterRow < transactions_.size()) {
                const Transaction& transaction = transactions_.at(index.row() - kReservedFilterRow);
                if (transaction.description.startsWith("!!!") or transaction.description.startsWith("[R]")) {
                    if (index.column() == 1) {
                        return QColor(Qt::red);
                    }
                }
            }
        } break;
        case Qt::DisplayRole: {
            Q_ASSERT(index.row() < kReservedFilterRow + transactions_.size() + 1);
            if (index.row() < kReservedFilterRow) {
                return QVariant();
            }
            const Transaction& transaction = index.row() - kReservedFilterRow == transactions_.size()?
                                             sum_transaction_ : transactions_.at(index.row() - kReservedFilterRow);
            switch (index.column()) {
                case 0: return transaction.date_time.toString(kDateTimeFormat);
                case 1: return transaction.description;
                case 2: // Fall through
                case 3: // Fall through
                case 4: // Fall through
                case 5: return transaction.toString(kAccountTypes.at(index.column() - 2));
            } break;
        } break;
        default:
          break;
    }

    return QVariant();
}

Transaction BookModel::getTransaction(const QModelIndex& model_index) const {
    int index = model_index.row() - kReservedFilterRow;
    if (index >= 0 and index < transactions_.size()) {
        return transactions_.at(index);
    } else {
        return Transaction();
    }
}

QList<Transaction> BookModel::getTransactions(const QModelIndexList& index_list) const {
    QSet<int> selected_indexes;
    for (QModelIndex model_index : index_list) {
        int index = model_index.row() - kReservedFilterRow;
        if (index < 0 or index >= transactions_.size()) {
            qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << model_index;
            continue;
        }
        selected_indexes.insert(index);
    }
    QList<Transaction> result;
    for (int index : selected_indexes) {
        result.append(transactions_.at(index));
    }
    return result;
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

//bool BookModel::hasChildren(const QModelIndex &parent) const {
//  // FIXME: Implement me!
//}

//bool BookModel::canFetchMore(const QModelIndex& /* parent */) const {
//    return row_count_ < kReservedFilterRow + transactions_.size() + /* sum_transaction */ 1;
//}

//void BookModel::fetchMore(const QModelIndex& /* parent */) {
//    const int remainder = int(kReservedFilterRow + transactions_.size() + 1) - row_count_;
//    const int itemsToFetch = qMin(kBatchSize, remainder);
//    if (itemsToFetch <= 0) {
//        return;
//    }
//    beginInsertRows(QModelIndex(), row_count_, row_count_ + itemsToFetch - 1);
//    row_count_ += itemsToFetch;
//    endInsertRows();

//    emit numberPopulated(path, start, itemsToFetch, int(fileList.size()));
//}

//bool BookModel::setData(const QModelIndex &index, const QVariant &value, int role) {
//  if (data(index, role) != value) {
//    // FIXME: Implement me!
//    emit dataChanged(index, index, QVector<int>() << role);
//    return true;
//  }
//  return false;
//}

//Qt::ItemFlags BookModel::flags(const QModelIndex &index) const {
//  if (!index.isValid())
//    return Qt::NoItemFlags;

//  return Qt::ItemIsEditable; // FIXME: Implement me!
//}

//bool BookModel::insertRows(int row, int count, const QModelIndex &parent) {
//  beginInsertRows(parent, row, row + count - 1);
//  // FIXME: Implement me!
//  endInsertRows();
//}

//bool BookModel::insertColumns(int column, int count, const QModelIndex &parent) {
//  beginInsertColumns(parent, column, column + count - 1);
//  // FIXME: Implement me!
//  endInsertColumns();
//}

//bool BookModel::removeRows(int row, int count, const QModelIndex &parent) {
//  beginRemoveRows(parent, row, row + count - 1);
//  // FIXME: Implement me!
//  endRemoveRows();
//}

//bool BookModel::removeColumns(int column, int count, const QModelIndex &parent) {
//  beginRemoveColumns(parent, column, column + count - 1);
//  // FIXME: Implement me!
//  endRemoveColumns();
//}
