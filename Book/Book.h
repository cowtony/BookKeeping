#ifndef BOOK_H
#define BOOK_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include <QtSql>
#include "transaction.h"
#include "account.h"

class BOOKSHARED_EXPORT Book {
public:
  // The constructor will create a instance with opened database.
  explicit Book(const QString& dbPath);
  ~Book();

  Book (const Book&) = delete;            // Book object is not copiable.
  Book& operator=(const Book&) = delete;  // Book object is not assignable.

  void closeDatabase();

  // Transactions
  bool dateTimeExist(const QDateTime& dt) const;
  bool insertTransaction(const Transaction& transaction) const;
  Transaction queryTransaction(const QDateTime& date_time) const;  // Not used.
  QList<Transaction> queryTransactions(const TransactionFilter& filter = TransactionFilter()) const;
  QList<FinancialStat> getSummaryByMonth(const QDateTime& p_endDateTime = QDateTime(QDate(2100, 12, 31), QTime(0, 0, 0))) const;
  void removeTransaction(const QDateTime& p_dateTime) const;

  // Account
  QList<Account> queryAllAccountsFrom(QList<Account::Type> account_types = {}) const;
  Currency::Type queryCurrencyType(const Account& account) const;
  QStringList    queryCategories  (Account::Type account_type) const;
  QList<Account> queryAccounts(Account::Type account_type, const QString& category) const;
  QStringList    queryAccountNamesByLastUpdate(Account::Type account_type, const QString& category, const QDateTime& date_time) const;
  bool updateAccountComment(const Account& account, const QString& comment) const;

  bool insertCategory(const QString& tableName, const QString& category) const;
  bool removeCategory(const QString& tableName, const QString& category) const;
  bool categoryExist (const QString& tableName, const QString& category) const;
  bool renameCategory(const QString& tableName, const QString& category, const QString& newCategory) const;
  bool insertAccount (const Account& account) const;
  bool removeAccount (const Account& account) const;
  bool accountExist  (const Account& account) const;
  bool moveAccount   (const Account& account, const Account& newAccount) const;

  QDateTime getFirstTransactionDateTime() const;
  QDateTime getLastTransactionDateTime() const;

  static QString getLastExecutedQuery(const QSqlQuery& query);

private:
  QSqlDatabase database_;

  QDateTime start_time_;
  void logUsageTime();

  bool logging(const QSqlQuery& query) const; // Log all the modifier actions
};

//BOOKSHARED_EXPORT extern Book g_book;

#endif // BOOK_H
