#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QDateTime>
#include <QMap>

#include "money.h"
#include "account.h"

class Transaction {
public:
    explicit Transaction(const QDateTime& date_time = QDateTime(), const QString& description = "");

    void operator +=(const Transaction& p_transaction);

    // Setters:
    void clear();
    void clear(Account::Type tableType);
    void addMoney(QSharedPointer<Account> account, const QString& household, Money money);  // This should be the only setter.

    // Getters:
            HouseholdMoney getHouseholdMoney(const Account& account) const;
    virtual HouseholdMoney getHouseholdMoney(Account::Type account_type, const QString& category_name, const QString& account_name) const;
    virtual QList<QPair<QSharedPointer<Account>, HouseholdMoney>> getAccounts() const;
    Money getCheckSum() const;
    QStringList validate() const;  // Return error messages
    QString toString(Account::Type account_type) const;

    QDateTime date_time;
    QString description;
    int id;

protected:
    QList<QPair<QSharedPointer<Account>, HouseholdMoney>> getAccounts(Account::Type account_type) const;
    Transaction operator + (Transaction transaction) const;
    bool contains(const Account& account) const;
    HouseholdMoney getXXXContributedCapital() const;  // not used yet

    //   <account_type, <category_name, <account_name, <account_ptr, household_money>>>>
    QHash<Account::Type, QHash<QString, QHash<QString, QPair<QSharedPointer<Account>, HouseholdMoney>>>> data_;
};

struct TransactionFilter : public Transaction {
    TransactionFilter();

    TransactionFilter& addAccount(QSharedPointer<Account> account);
    TransactionFilter& startTime(const QDateTime& start_time);
    TransactionFilter& endTime(const QDateTime& start_time);
    TransactionFilter& setDescription(const QString& description);
    TransactionFilter& useOr();
    TransactionFilter& useAnd();
    TransactionFilter& orderByAscending();
    TransactionFilter& orderByDescending();
    TransactionFilter& setLimit(int lim);

    QDateTime end_date_time = QDateTime(QDate(2200, 01, 01), QTime(23, 59, 59));
    bool use_or = false;
    bool ascending_order = true;
    int limit = 99999999;
    QString timeZone;
};

// TODO: merge this into Transaction
class FinancialStat : public Transaction {
public:
    explicit FinancialStat();

    // Getters:
    virtual HouseholdMoney getHouseholdMoney(Account::Type account_type, const QString& category_name, const QString& account_name) const override;
    virtual QList<QPair<QSharedPointer<Account>, HouseholdMoney>> getAccounts() const override;

    void cumulateRetainedEarning();
    void cumulateCurrencyError(const QDateTime& new_date_time);  // Change date so that the currencyError is calculated and counted.
    void cumulateTransaction(const Transaction& transaction);

private:
    HouseholdMoney retained_earning_;
    Money currency_error_;  // Error caused by currency rate different from day to day.
    Money cumulated_check_sum_;  // The sum of each transaction's check_sum, abs(each) normally smaller than $0.005
};

#endif // TRANSACTION_H
