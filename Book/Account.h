#ifndef ACCOUNT_H
#define ACCOUNT_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include <QString>
#include "Currency.h"

struct BOOKSHARED_EXPORT Account {
  typedef enum {Asset, Liability, Revenue, Expense, Equity} Type;
  static const QMap<Type, QString> kTableName;

  explicit Account(Type table, const QString& category, const QString& name);
  explicit Account(const QString& tableName, const QString& category, const QString& name);

  Type    type;
  QString category;
  QString name;

  QString typeName() const;
  QString getFinancialStatementName() const; // Income Statement, Balance Sheet, Cash Flow

  bool operator  <(const Account& account) const;
  bool operator ==(const Account& account) const;

private:
  int id_;  // Primary key in database, not used.
};

#endif // ACCOUNT_H
