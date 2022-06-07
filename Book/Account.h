#ifndef ACCOUNT_H
#define ACCOUNT_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include <QString>
#include "currency.h"

struct BOOKSHARED_EXPORT Account {
  typedef enum {Asset, Liability, Revenue, Expense, Equity} Type;
  static const QMap<Type, QString> kTableName;

  explicit Account(Type account_type, const QString& category, const QString& name, const QString& comment = "")
    : type(account_type), category(category), name(name), comment(comment) {}
  explicit Account(const QString& account_type, const QString& category, const QString& name, const QString& comment = "");

  Type    type;
  QString category;
  QString name;
  QString comment;

  QString typeName() const;
  QString getFinancialStatementName() const; // Income Statement, Balance Sheet, Cash Flow

  bool operator  <(const Account& account) const;
  bool operator ==(const Account& account) const;

private:
  int id_;  // Primary key in database, not used.
};

struct BOOKSHARED_EXPORT AssetAccount : public Account {
  explicit AssetAccount(Type account_type, const QString& category, const QString& name, const QString& comment = "", bool is_investment = false)
    : Account(account_type, category, name, comment),
      is_investment(is_investment) {}

  bool is_investment;
};

BOOKSHARED_EXPORT std::shared_ptr<Account> FactoryCreateAccount(Account::Type account_type, const QString& category, const QString& name, const QString& comment = "");

#endif // ACCOUNT_H
