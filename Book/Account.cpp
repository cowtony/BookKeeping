#include "Account.h"

const QMap<Account::Type, QString> Account::kTableName =
    {{Asset, "Asset"},
     {Liability, "Liability"},
     {Revenue, "Revenue"},
     {Expense, "Expense"},
     {Equity, "Equity"}};

Account::Account(Type account_type, const QString& category, const QString& name, const QString& comment)
  : type(account_type), category(category), name(name), comment(comment) {}

Account::Account(const QString& account_type, const QString& category, const QString& name, const QString& comment)
  : category(category), name(name), comment(comment) {
  if (kTableName.values().contains(account_type)) {
    type = kTableName.key(account_type);
  } else {
    qDebug() << Q_FUNC_INFO << "table name doesn't exist!" << account_type;
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
