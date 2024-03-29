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
    bool insertTransaction(int user_id, const Transaction& transaction, bool ignore_error = false);
    static QString getQueryTransactionsQueryStr(int user_id, const TransactionFilter& filter = TransactionFilter());
    QList<Transaction> queryTransactions(int user_id, const TransactionFilter& filter = TransactionFilter()) const;
    Transaction getTransaction(int transaction_id) const;
    bool removeTransaction(int transaction_id);
    QDateTime getFirstTransactionDateTime() const;
    QDateTime getLastTransactionDateTime() const;

    // Accounts
    QList<AssetAccount> getInvestmentAccounts(int user_id) const;
    Currency::Type queryCurrencyType(int user_id, Account::Type account_type, const QString& category_name, const QString& account_name) const;
    QList<QSharedPointer<Account>> queryAccountNamesByLastUpdate(int user_id, Account::Type account_type, const QString& category_name, const QDateTime& date_time) const;
    QStringList getHouseholds(int user_id) const;

    // Account Management
    QSharedPointer<Account> getAccount(int user_id, Account::Type account_type, const QString& category_name, const QString& account_name) const;
    QSharedPointer<Account> insertAccount(int user_id, Account::Type account_type, const QString& category_name, const QString& account_name) const;
    QString setInvestment(int user_id, const AssetAccount& asset, bool is_investment); // Return the error string, empty if no error. // TODO: Use StatusOr<>
    bool updateAccountComment(int account_id, const QString& comment) const;
    QString renameAccount(int user_id, const Account& old_account, const QString& account_name); // Return the error string, empty if no error. // TODO: Use StatusOr<>
    // TODO: Add merge account feature.
    bool removeAccount (int account_id) const;

    // Category Management
    QList<QSharedPointer<Account>> getCategories(int user_id, Account::Type account_type) const;
    QSharedPointer<Account> getCategory(int user_id, Account::Type account_type, const QString& category_name) const;
    QSharedPointer<Account> insertCategory(int user_id, Account::Type account_type, const QString& category_name) const;
    bool renameCategory(int user_id, Account::Type account_type, const QString& category_name, const QString& new_category_name) const;
    bool removeCategory(int category_id) const;


    QList<QSharedPointer<Account>> queryAllAccounts(int user_id) const;
    QList<QSharedPointer<Account>> queryAllCategories(int user_id) const;

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
    static void populateTransactionDataFromQuery(Transaction& transaction, const QSqlQuery& query);

    QDateTime start_time_;
};

#endif // BOOK_H
