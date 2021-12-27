#include "Account.h"

const QMap<Account::Type, QString> Account::kTableName =
    {{Asset, "Asset"},
     {Liability, "Liability"},
     {Revenue, "Revenue"},
     {Expense, "Expense"},
     {Equity, "Equity"}};

Account::Account(Type table, const QString& category, const QString& name) : type(table), category(category), name(name) {}

Account::Account(const QString& tableName, const QString& category, const QString& name) : category(category), name(name) {
  if (kTableName.values().contains(tableName)) {
    type = kTableName.key(tableName);
  } else {
    qDebug() << Q_FUNC_INFO << "table name doesn't exist!" << tableName;
  }
}

QString Account::typeName() const {
  return kTableName.value(type);
}

QString Account::getFinancialStatementName() const {
  switch (type) {
    case Revenue:
    case Expense:
      return "Income Statement";
    case Asset:
    case Liability:
    case Equity:
      return "Balance Sheet";
    // TODO: Add case for return "Cash Flow";
  }
  return QString();
}

bool Account::operator <(const Account &p_account) const {
  if (category == p_account.category) {
    return category + name < p_account.category + p_account.name;
  } else {
    return category < p_account.category;
  }
}

bool Account::operator ==(const Account &account) const {
  return account.type == type and account.category == category and account.name == name;
}
