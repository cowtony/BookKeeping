#include "transactions_model.h"
#include "home_window.h"

TransactionsModel::TransactionsModel(QObject *parent)
    : QSqlQueryModel(parent),
      db_(static_cast<HomeWindow*>(parent)->book.db),
      book_(static_cast<HomeWindow*>(parent)->book),
      user_id_(static_cast<HomeWindow*>(parent)->user_id) {
    refresh();
}

int TransactionsModel::rowCount(const QModelIndex &parent) const {
    return QSqlQueryModel::rowCount(parent) + 1;  // Add one row for the total
}

QVariant TransactionsModel::data(const QModelIndex &index, int role) const {
    if (role == Qt::DisplayRole) {
        if (index.row() == rowCount() - 1) {  // The sum row.
            switch (index.column()) {
            case 1: return sum_transaction_.description;
            case 2: return sum_transaction_.toString(Account::Expense);
            case 3: return sum_transaction_.toString(Account::Revenue);
            case 4: return sum_transaction_.toString(Account::Asset);
            case 5: return sum_transaction_.toString(Account::Liability);
            default: return QVariant();
            }
        } else if (index.column() == 0) {
            // Read the UTC timestamp and time zone to print time zoned local time string
            qint64 utcTimestamp = QSqlQueryModel::data(index, role).toLongLong();
            QDateTime dateTime = QDateTime::fromSecsSinceEpoch(utcTimestamp, QTimeZone("UTC"));

            QString timeZoneId = QSqlQueryModel::data(this->index(index.row(), kTimeZoneColumnIndex), role).toString();
            QTimeZone timeZone(timeZoneId.toUtf8());

            // If the time zone ID is invalid, fall back to the system's local time zone
            if (!timeZoneId.isEmpty() && timeZone.isValid()) {
                dateTime = dateTime.toTimeZone(timeZone);
            }
            return dateTime.toString("yyyy-MM-ddTHH:mmttt");
        } else if (index.column() >= 2 && index.column() <= 5) {
            return QSqlQueryModel::data(index).toString().replace(R"(\n)", "\n");
        }
    } else if (role == Qt::FontRole) {
        if (index.row() == rowCount() - 1) {  // The sum row.
            QFont font;
            font.setBold(true);
            return font;
        }
    } else if (role == Qt::BackgroundRole) {
        if (index.row() < rowCount() - 1 && index.column() == 1) {
            // Descriptions need pay attention.
            QString description = QSqlQueryModel::data(createIndex(index.row(), 1)).toString();
            if (description.startsWith("!!!") || description.startsWith("[R]")) {
                return QColor(Qt::red);
            }
        }
    }
    return QSqlQueryModel::data(index, role);
}

QString TransactionsModel::getDisplayRoleText(int row, int col) const {
    return data(createIndex(row, col), Qt::DisplayRole).toString();
}

void TransactionsModel::setFilter(const TransactionFilter& filter) {
    filter_ = filter;
    refresh();
}

void TransactionsModel::refresh() {
    QString queryString = Book::getQueryTransactionsQueryStr(user_id_, filter_);
    qDebug().noquote() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << "Dashboard query string:\n                        " << queryString;
    setQuery(queryString, db_);

    // TODO: Try to store all the Transactions here as well, so that `getTransaction()` don't need to query one more time.
    sum_transaction_.clear();
    for (int row = 0; row < QSqlQueryModel::rowCount(); ++row) {
        sum_transaction_ += getTransaction(row);
    }
    sum_transaction_.description = "Sum:";
}

Transaction TransactionsModel::getTransaction(int row) {
    return book_.getTransaction(data(createIndex(row, kTransactionIdColumnIndex)).toInt());
}

