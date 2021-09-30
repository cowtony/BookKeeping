#include "Account.h"

const QMap<Account::TableType, QString> Account::TableName =
    {{Asset, "Asset"},
     {Liability, "Liability"},
     {Revenue, "Revenue"},
     {Expense, "Expense"},
     {Equity, "Equity"}};

Account::Account(TableType table, const QString& category, const QString& name) : table_(table), category_(category), name_(name) {}

Account::Account(const QString& tableName, const QString& category, const QString& name) : category_(category), name_(name) {
  if (TableName.values().contains(tableName)) {
    table_ = TableName.key(tableName);
  } else {
    qDebug() << Q_FUNC_INFO << "table name doesn't exist!" << tableName;
  }
}

QString Account::getTableName() const {
  return TableName.value(table_);
}

QString Account::getFinancialStatementName() const {
  switch (table_) {
    case Revenue:
    case Expense:
      return "Income Statement";
    case Asset:
    case Liability:
    case Equity:
      return "Balance Sheet";
    // TODO: Add case for return "Cash Flow";
  }
}

bool Account::operator <(const Account &p_account) const {
  if (category_ == p_account.category_) {
    return category_ + name_ < p_account.category_ + p_account.name_;
  } else {
    return category_ < p_account.category_;
  }
}

bool Account::operator ==(const Account &account) const {
  return account.table_ == table_ and account.category_ == category_ and account.name_ == name_;
}
