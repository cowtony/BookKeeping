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

    MoneyArray getMoneyArray(const Account& account) const;
    void       addMoneyArray(const Account& account, const MoneyArray& moneyArray);

    void setData(const QJsonObject& json);

    bool accountExist(const Account& account) const;
    Money getCheckSum() const;
    QStringList validate() const;  // Return error message
    QString toString(Account::Type table_type) const;
    QJsonObject toJson() const;

    QList<Account> getAccounts() const;
    QList<Account> getAccounts(Account::Type tableType) const;

    MoneyArray getRetainedEarnings() const;
    MoneyArray getXXXContributedCapital() const;  // not used yet

    QDateTime date_time;
    QString description;

  protected:

  private:
    // Key 0: table type, Key 1: Category, Key 2: Account Name
    // TODO: Key 3: User Name
    // TODO: Use QHash than QMap
    QMap<Account::Type, QMap<QString, QMap<QString, MoneyArray>>> data_;
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
    TransactionFilter& setLimit(int limit);

    QDateTime end_date_time_ = QDateTime(QDate(2200, 01, 01), QTime(23, 59, 59));
    bool use_or_ = false;
    bool ascending_order_ = true;
    int limit_ = INT_MAX;
};

// TODO: merge this into Transaction
class FinancialStat : public Transaction {
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
