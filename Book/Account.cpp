#include "Account.h"

const QMap<Account::TableType, QString> Account::TableName = {{Asset, "Asset"},
                                                              {Liability, "Liability"},
                                                              {Revenue, "Revenue"},
                                                              {Expense, "Expense"},
                                                              {Equity, "Equity"}};

Account::Account(const TableType &table, const QString &category, const QString &name) : m_table(table), m_category(category), m_name(name)
{
}

Account::Account(const QString &tableName, const QString &category, const QString &name) : m_category(category), m_name(name)
{
    if (TableName.values().contains(tableName))
        m_table = TableName.key(tableName);
    else {
        qDebug() << Q_FUNC_INFO << "table name doesn't exist!" << tableName;
    }
}

QString Account::getTableName() const {
  return TableName.value(m_table);
}

QString Account::getFinancialStatementName() const {
  switch (m_table) {
    case Revenue:
    case Expense:
      return "Income Statement";
    case Asset:
    case Liability:
    case Equity:
      return "Balance Sheet";
    default:
      return "Cash Flow";
  }
}

bool Account::operator <(const Account &p_account) const {
    if (m_table == p_account.m_category)
        return m_category + m_name < p_account.m_category + p_account.m_name;
    else
        return m_table < p_account.m_category;
}

bool Account::operator ==(const Account &account) const
{
    return account.m_table == m_table and account.m_category == m_category and account.m_name == m_name;
}
