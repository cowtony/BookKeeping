#ifndef BOOK_H
#define BOOK_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include <QtSql>
#include "Transaction.h"
#include "Account.h"

const QString DATE_TIME_FORMAT = "yyyy-MM-dd HH:mm";

class BOOKSHARED_EXPORT Book {
public:
  explicit Book();
  ~Book();

  bool openDatabase(const QString& dbPath);
  void closeDatabase();

  // Transactions
  bool dateTimeExist(const QDateTime& dt) const;
  Transaction getTransaction(const QDateTime& dt);
  bool insertTransaction(const Transaction& t) const;
  QList<Transaction> queryTransactions(const QDateTime& startTime,
                                       const QDateTime& endTime,
                                       const QString& description,
                                       const QList<Account>& accounts,
                                       bool ascending = true,
                                       bool accountUnion = false) const;
  QList<FinancialStat> getSummaryByMonth(const QDateTime& p_endDateTime = QDateTime(QDate(2100, 12, 31))) const;
  void removeTransaction(const QDateTime& p_dateTime) const;

  // Account
  Currency_e  getCurrencyType(const Account& account) const;
  QStringList getCategories  (Account::TableType p_tableType) const;
  QStringList getAccountNames(Account::TableType p_tableType, const QString& p_category) const;
  QStringList getAccountNamesByLastUpdate(Account::TableType p_tableType, const QString& p_category, const QDateTime& p_dateTime) const;
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
  QSqlDatabase m_database;

  QDateTime m_startTime;
  void logUsageTime();

  bool logging(const QSqlQuery& query) const; // Log all the modifier actions
};

BOOKSHARED_EXPORT extern Book g_book;

#endif // BOOK_H
