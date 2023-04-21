#include "transactions_model.h"
#include "home_window.h"

TransactionsModel::TransactionsModel(QObject *parent)
    : QSqlQueryModel(parent),
      db_(static_cast<HomeWindow*>(parent)->book.db),
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
            case 2:
                return QJsonDocument(sum_transaction_.toJson().value("Expense").toObject()).toJson(QJsonDocument::Compact).toStdString().c_str();
            case 3: return QJsonDocument(sum_transaction_.toJson().value("Revenue").toObject()).toJson(QJsonDocument::Compact).toStdString().c_str();
            case 4: return QJsonDocument(sum_transaction_.toJson().value("Asset").toObject()).toJson(QJsonDocument::Compact).toStdString().c_str();
            case 5: return QJsonDocument(sum_transaction_.toJson().value("Liability").toObject()).toJson(QJsonDocument::Compact).toStdString().c_str();
            default: return QVariant();
            }
        }
        else if (index.column() <= 5 && index.column() >= 2) {
            QJsonObject json_obj = QJsonDocument::fromJson(QSqlQueryModel::data(index).toString().toUtf8()).object();
            QStringList lines;
            for (const QString& account_name : json_obj.keys()) {
                lines.append(QString(R"("%1": "%2")").arg(account_name).arg(json_obj.value(account_name).toString()));
            }
            return "{" + lines.join(",\n") + "}";
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
    for (const Account& account : filter_.getAccounts()) {
        if (!account.category.isEmpty()) {
            statements << QString(R"sql((detail->'%1'->>'%2|%3' NOT NULL))sql").arg(account.typeName(), account.category, account.name);
        }
    }

    QString query = QString(R"sql(SELECT
                                      date_time AS Timestamp,
                                      description AS Description,
                                      detail->'Expense'   AS Expense,
                                      detail->'Revenue'   AS Revenue,
                                      detail->'Asset'     AS Asset,
                                      detail->'Liability' AS Liability,
                                      transaction_id
                                  FROM book_transactions
                                  WHERE
                                      user_id = %1
                                      AND (date_time BETWEEN '%2' AND '%3')
                                      AND (description LIKE '%%4%')
                                      AND (%5)
                                  ORDER BY Timestamp %6
                                  LIMIT %7)sql")
        .arg(user_id_)
        .arg(filter_.date_time.toString(kDateTimeFormat))
        .arg(filter_.end_date_time.toString(kDateTimeFormat))
        .arg(filter_.description)
        .arg(statements.empty()? "TRUE" : statements.join(filter_.use_or? " OR " : " AND "))
        .arg(filter_.ascending_order? "ASC" : "DESC")
        .arg(filter_.limit);

    setQuery(query, db_);

    sum_transaction_.clear();
    for (int row = 0; row < QSqlQueryModel::rowCount(); ++row) {
        sum_transaction_ += getTransaction(row);
    }
    sum_transaction_.description = "Sum:";
}

Transaction TransactionsModel::getTransaction(int row) {
    static const QList<QString> kTypeName = {"Expense", "Revenue", "Asset", "Liability"};
    QJsonObject json;
    for (int i = 0; i < kTypeName.size(); ++i) {
        QJsonDocument json_document = QJsonDocument::fromJson(data(createIndex(row, i + 2)).toString().toUtf8());
        json[kTypeName[i]] = json_document.object();
    }
    Transaction transaction(data(createIndex(row, 0)).toDateTime(), data(createIndex(row, 1)).toString());
    transaction.id = data(createIndex(row, 6)).toInt();
    transaction.addData(json);
    return transaction;
}

