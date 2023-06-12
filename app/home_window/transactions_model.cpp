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
        if (index.row() == rowCount() - 1) {  // Total row.
            switch (index.column()) {
            case 1: return sum_transaction_.description;
            case 2: return sum_transaction_.toString(Account::Expense);
            case 3: return sum_transaction_.toString(Account::Revenue);
            case 4: return sum_transaction_.toString(Account::Asset);
            case 5: return sum_transaction_.toString(Account::Liability);
            default: return QVariant();
            }
        } else if (index.column() <= 5 && index.column() >= 2) {
            return QSqlQueryModel::data(index).toString().replace(R"(\n)", "\n");
        }
    } else if (role == Qt::FontRole) {
        if (index.row() == rowCount() - 1) {  // Total row.
            QFont font;
            font.setBold(true);
            return font;
        }
    } else if (role == Qt::BackgroundRole) {
        if (index.row() < rowCount() - 1) {
            // Descriptions need pay attention.
            QString description = QSqlQueryModel::data(createIndex(index.row(), 1)).toString();
            if (description.startsWith("!!!") || description.startsWith("[R]")) {
                return QColor(Qt::red);
            }
        }
    }
    return QSqlQueryModel::data(index, role);
}

QVariant TransactionsModel::data(int row, int col) const {
    return data(createIndex(row, col));
}

void TransactionsModel::setFilter(const TransactionFilter& filter) {
    filter_ = filter;
    refresh();
}

void TransactionsModel::refresh() {
    QStringList statements;
    for (const auto& [account, household_money] : filter_.getAccounts()) {
        if (!account->categoryName().isEmpty()) {
            statements << QString(R"sql((%1 LIKE '%%2|%3%'))sql").arg(account->typeName(), account->categoryName(), account->accountName());
        }
    }

    QString query = QString(R"sql(SELECT Timestamp, Description, Expense, Revenue, Asset, Liability, transaction_id
                                  FROM   transactions_view
                                  WHERE
                                      user_id = %1
                                      AND (Timestamp BETWEEN '%2' AND '%3')
                                      AND (Description LIKE '%%4%')
                                      AND (%5)
                                  ORDER BY Timestamp DESC
                                  LIMIT %7)sql")
        .arg(user_id_)
        .arg(filter_.date_time.toString(kDateTimeFormat))
        .arg(filter_.end_date_time.toString(kDateTimeFormat))
        .arg(filter_.description)
        .arg(statements.empty()? "TRUE" : statements.join(filter_.use_or? " OR " : " AND "))
        .arg(200);
    setQuery(query, db_);

    sum_transaction_.clear();
    for (int row = 0; row < QSqlQueryModel::rowCount(); ++row) {
        sum_transaction_ += getTransaction(row);
    }
    sum_transaction_.description = "Sum:";
}

Transaction TransactionsModel::getTransaction(int row) {
    return book_.getTransaction(data(createIndex(row, 6)).toInt());
}

