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

const QString DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm";

class BOOKSHARED_EXPORT Transaction {
public:
  explicit Transaction(const QDateTime& date_time = QDateTime(), const QString& description = "");

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

  QDateTime date_time_;
  QString description_;

protected:

private:
  // Key 0: table type, Key 1: Category, Key 2: Account Name
  QMap<Account::TableType, QMap<QString, QMap<QString, MoneyArray>>> data_;
};

struct BOOKSHARED_EXPORT TransactionFilter : public Transaction {
  TransactionFilter(const QDateTime& start_time = QDateTime(QDate(1990, 05, 25), QTime(0, 0, 0)),
                    const QDateTime& end_time = QDateTime(QDate(2200, 01, 01), QTime(23, 59, 59)),
                    const QString& description = "",
                    const QList<Account>& accounts = {},
                    bool use_union = false,
                    bool ascending_order = true);

  void addAccount(const Account& account);

  QDateTime end_date_time_;
  bool use_union_ = false;
  bool ascending_order_ = true;
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
