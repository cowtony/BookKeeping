#ifndef ACCOUNT_H
#define ACCOUNT_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include <QString>
#include "Currency.h"

struct BOOKSHARED_EXPORT Account
{
    typedef enum{Asset, Liability, Revenue, Expense, Equity} TableType;
    static const QMap<TableType, QString> TableName;

    explicit Account(TableType table, const QString& category, const QString& name);
    explicit Account(const QString& tableName, const QString& category, const QString& name);

    TableType m_table;
    QString   m_category;
    QString   m_name;

    QString getTableName() const;
    QString getFinancialStatementName() const; // Income Statement, Balance Sheet, Cash Flow

    bool operator  <(const Account& account) const;
    bool operator ==(const Account& account) const;
};

#endif // ACCOUNT_H
