#ifndef BOOK_H
#define BOOK_H

#include <QtSql>
#include "transaction.h"
#include "account.h"

class Book {
  public:
    // The constructor will create a instance with opened database.
    explicit Book(const QString& dbPath);
    ~Book();

    Book (const Book&) = delete;            // Book object is not copiable.
    Book& operator=(const Book&) = delete;  // Book object is not assignable.

    QSqlDatabase db;
    void closeDatabase();

    // Transactions
    bool insertTransaction(int user_id, const Transaction& transaction, bool ignore_error = false) const;
    QList<Transaction> queryTransactions(const TransactionFilter& filter = TransactionFilter()) const;
    QList<FinancialStat> getSummaryByMonth(const QDateTime& p_endDateTime = QDateTime(QDate(2100, 12, 31), QTime(0, 0, 0))) const;
    void removeTransaction(int transaction_id) const;

    // Account
    QList<std::shared_ptr<Account>> queryAllAccountsFrom(QList<Account::Type> account_types = {}) const;
    QStringList queryAccounts(Account::Type account_type, const QString& category) const;
    QList<AssetAccount> getInvestmentAccounts() const;
    Currency::Type queryCurrencyType(const Account& account) const;
    QStringList    queryCategories  (Account::Type account_type) const;
    QStringList    queryAccountNamesByLastUpdate(Account::Type account_type, const QString& category, const QDateTime& date_time) const;
    bool updateAccountComment(const Account& account, const QString& comment) const;
    QString setInvestment(const AssetAccount& asset, bool is_investment) const;
    bool IsInvestment(const Account& account) const;

    bool insertCategory(Account::Type account_type, const QString& category) const;
    bool categoryExist (Account::Type account_type, const QString& category) const;
    bool renameCategory(Account::Type account_type, const QString& category, const QString& newCategory) const;
    bool insertAccount (const Account& account) const;
    bool removeAccount (const Account& account) const;
    bool accountExist  (const Account& account) const;
    // Return the error string, empty if no error.
    QString moveAccount(const Account& old_account, const Account& new_account) const;

    QDateTime getFirstTransactionDateTime() const;
    QDateTime getLastTransactionDateTime() const;

    static QString getLastExecutedQuery(const QSqlQuery& query);

    // Login related
    bool updateLoginTime(int user_id) const;
    int getLastLoggedInUserId() const;

  private:
    QDateTime start_time_;

    void logUsageTime();
    void reduceLoggingRows();
    bool Logging(const QSqlQuery& query) const; // Log all the modifier actions
};

#endif // BOOK_H
