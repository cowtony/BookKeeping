#ifndef TRANSACTION_H
#define TRANSACTION_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include <QDateTime>
#include <QMap>
#include "Currency.h"
#include "Money.h"
#include "Account.h"


class BOOKSHARED_EXPORT Transaction {
public:
  explicit Transaction();

  QDateTime date_time_;
  QString description_;


  Transaction operator + (Transaction p_transaction) const;
  void        operator +=(const Transaction& p_transaction);

  void clear();
  void clear(Account::TableType tableType);

  MoneyArray getMoneyArray(const Account& account) const;
  void       addMoneyArray(const Account& account, const MoneyArray& moneyArray);
  void stringToData(Account::TableType tableType, const QString& data);

  bool accountExist(const Account& account) const;
  Money getCheckSum() const;
  QStringList validation() const;  // Return error message
  QString dataToString(Account::TableType p_tableType) const;

  QList<Account> getAccounts() const;
  QList<Account> getAccounts(Account::TableType tableType) const;

  MoneyArray getRetainedEarnings() const;
  MoneyArray getXXXContributedCapital() const;  // not used yet
protected:

private:
  // Key 0: table type, Key 1: Category, Key 2: Account Name
  QMap<Account::TableType, QMap<QString, QMap<QString, MoneyArray>>> data_;
};

// TODO: merge this into Transaction
class BOOKSHARED_EXPORT FinancialStat : public Transaction {
public:
  explicit FinancialStat();

  MoneyArray retainedEarnings;
  MoneyArray transactionError;

  MoneyArray getMoneyArray(const Account& account) const;
  QList<Account> getAccounts() const;

  // Change date so that the currencyError is calculated and counted.
  void changeDate(const QDate& newDate);

private:
  MoneyArray currencyError;
};

#endif // TRANSACTION_H
