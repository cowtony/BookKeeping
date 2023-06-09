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
    QDateTime getFirstTransactionDateTime() const;
    QDateTime getLastTransactionDateTime() const;

    // Accounts
    QList<std::shared_ptr<Account>> queryAllAccounts(int user_id) const;  // TODO: use QSharedPointer

    QList<AssetAccount> getInvestmentAccounts(int user_id) const;
    Currency::Type queryCurrencyType(int user_id, const Account& account) const;
    QStringList    queryCategories  (int user_id, Account::Type account_type) const;
    QStringList    queryAccountNamesByLastUpdate(int user_id, Account::Type account_type, const QString& category, const QDateTime& date_time) const;
    bool updateAccountComment(int user_id, const Account& account, const QString& comment) const;
    QString setInvestment(int user_id, const AssetAccount& asset, bool is_investment) const;

    bool insertCategory(int user_id, Account::Type account_type, const QString& category_name) const;
    bool categoryExist (int user_id, Account::Type account_type, const QString& category_name) const;
    bool renameCategory(int user_id, Account::Type account_type, const QString& category_name, const QString& new_category_name) const;
    bool insertAccount (int user_id, const Account& account) const;
    bool removeAccount (int user_id, const Account& account) const;
    bool accountExist  (int user_id, const Account& account) const;
    QString moveAccount(int user_id, const Account& old_account, const Account& new_account) const; // Return the error string, empty if no error. // TODO: Use StatusOr<>

    // Login related
    bool updateLoginTime(int user_id) const;
    int getLastLoggedInUserId() const;

  private:
    void logUsageTime();
    void reduceLoggingRows();
    bool Logging(const QSqlQuery& query) const; // Log all the modifier actions
    QStringList queryAccounts(int user_id, Account::Type account_type, const QString& category) const;
    bool IsInvestment(int user_id, const Account& account) const;
    static QString getLastExecutedQuery(const QSqlQuery& query);

    QDateTime start_time_;
};

#endif // BOOK_H
