#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QDateTime>
#include <QMap>

#include "money.h"
#include "account.h"

constexpr const char* kDateTimeFormat = "yyyy-MM-dd HH:mm";

class Transaction {
public:
    explicit Transaction(const QDateTime& date_time = QDateTime(), const QString& description = "");

    Transaction operator + (Transaction p_transaction) const;
    void        operator +=(const Transaction& p_transaction);

    void clear();
    void clear(Account::Type tableType);

    HouseholdMoney getHouseholdMoney(const Account& account) const;
    void           addHouseholdMoney(const Account& account, const HouseholdMoney& household_money);

    bool accountExist(const Account& account) const;
    Money getCheckSum() const;
    QStringList validate() const;  // Return error message
    QString toString(Account::Type account_type) const;

    QList<Account> getAccounts() const;
    QList<Account> getAccounts(Account::Type tableType) const;

    HouseholdMoney getRetainedEarnings() const;
    HouseholdMoney getXXXContributedCapital() const;  // not used yet

    QDateTime date_time;
    QString description;
    int id;

    // Key 0: table type, Key 1: Category, Key 2: Account Name, Key 3: Household Name
    // TODO: Use QHash than QMap
    QMap<Account::Type, QMap<QString, QMap<QString, HouseholdMoney>>> data_;

protected:

private:

};

struct TransactionFilter : public Transaction {
    TransactionFilter(const QList<Account>& accounts = {});

    TransactionFilter& addAccount(const Account& account);
    TransactionFilter& fromTime(const QDateTime& start_time);
    TransactionFilter& toTime(const QDateTime& start_time);
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
};

// TODO: merge this into Transaction
class FinancialStat : public Transaction {
  public:
    explicit FinancialStat();

    HouseholdMoney retainedEarnings;
    Money transactionError;

    HouseholdMoney getHouseholdMoney(const Account& account) const;
    QList<Account> getAccounts() const;

    // Change date so that the currencyError is calculated and counted.
    void changeDate(const QDate& newDate);

  private:
    Money currencyError;
};

#endif // TRANSACTION_H
