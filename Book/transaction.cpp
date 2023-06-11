#include "transaction.h"

Transaction::Transaction(const QDateTime& date_time, const QString& description)
    : date_time(date_time),
      description(description),
      id(-1),
      data_({{Account::Expense, {}}, {Account::Expense, {}}, {Account::Expense, {}}, {Account::Expense, {}}}) {}

Transaction Transaction::operator +(Transaction transaction) const {
    // dateTime is the maximum dateTime
    transaction.date_time = transaction.date_time > date_time? transaction.date_time : date_time;

    // merge description
    if (description.contains(transaction.description)) {
        transaction.description = description;
    } else if (!transaction.description.contains(description)) {
        transaction.description = description + "; " + transaction.description;
    }

    // Add up account
    for (const auto& [account_type, categories] : data_.asKeyValueRange()) {
        for (const auto& [category, accounts] : categories.asKeyValueRange()) {
            for (const auto& [account, households] : accounts.asKeyValueRange()) {
                for (const auto& [household, money] : households.asKeyValueRange()) {
                    transaction.data_[account_type][category][account][household] += money;
                    // Recursively remove empty key.
                    if (transaction.data_[account_type][category][account][household].isZero()) {
                        transaction.data_[account_type][category][account].remove(household);
                        if (transaction.data_[account_type][category][account].isEmpty()) {
                            transaction.data_[account_type][category].remove(account);
                            if (transaction.data_[account_type][category].isEmpty()) {
                                transaction.data_[account_type].remove(category);
                            }
                        }
                    }
                }
            }
        }
    }

    return transaction;
}

void Transaction::operator +=(const Transaction &t) {
  *this = *this + t;
}

void Transaction::clear() {
    date_time = QDateTime();
    description.clear();
    id = -1;
    data_ = {{Account::Expense, {}}, {Account::Expense, {}}, {Account::Expense, {}}, {Account::Expense, {}}};
}

void Transaction::clear(Account::Type tableType) {
    data_[tableType].clear();
}

bool Transaction::accountExist(const Account& account) const {
    if (data_.contains(account.type)) {
        if (data_.value(account.type).contains(account.category)) {
            if (data_.value(account.type).value(account.category).contains(account.name)) {
                return true;
            }
        }
    }
    return false;
}

Money Transaction::getCheckSum() const {
    Money sum(date_time.date());
    for (const auto& [account_type, categories] : data_.asKeyValueRange()) {
        for (const auto& [category, accounts] : categories.asKeyValueRange()) {
            for (const auto& [account, households] : accounts.asKeyValueRange()) {
                for (const auto& [household, money] : households.asKeyValueRange()) {
                    if (account_type == Account::Expense || account_type == Account::Asset) {
                        sum += money;
                    } else {
                        sum -= money;
                    }
                }
            }
        }
    }
    return sum;
}

QStringList Transaction::validate() const {
    QStringList errorMessage;
    if (description.isEmpty()) {
        errorMessage << "Description is empty.";
    }
    if (data_.value(Account::Asset).isEmpty() &&
        data_.value(Account::Expense).isEmpty() &&
        data_.value(Account::Revenue).isEmpty() &&
        data_.value(Account::Liability).isEmpty()) {
        errorMessage << "No account entries.";
    }
    Money sum = getCheckSum();
    if (sum.isZero()) {
        errorMessage << "The sum of the transaction is not zero: " + QString::number(sum.amount_);
    }

    return errorMessage;
}

QString Transaction::toString(Account::Type account_type) const {
    QStringList result;
    for (const auto& [category, accounts] : data_[account_type].asKeyValueRange()) {
        for (const auto& [account, households] : accounts.asKeyValueRange()) {
            for (const auto& [household, money] : households.asKeyValueRange()) {
                if (!money.isZero()) {
                    if (account_type == Account::Expense || account_type == Account::Revenue) {
                        result << QString("[%1|%2|%3: %4]").arg(category, account, household, money);
                    } else {
                        result << QString("[%1|%2: %3]").arg(category, account, money);
                    }
                }
            }
        }
    }
    return result.join("\n");
}

QList<Account> Transaction::getAccounts() const {
    QList<Account> all_ccounts;
    for (const Account::Type& tableType : {Account::Asset, Account::Expense, Account::Revenue, Account::Liability}) {
        for (const auto& [category, account] : data_.value(tableType).asKeyValueRange()) {
            for (const auto& [account_name, _] : account.asKeyValueRange()) {
                all_ccounts.push_back(Account(tableType, category, account_name));
            }
        }
    }
    return all_ccounts;
}

QList<Account> Transaction::getAccounts(Account::Type account_type) const {
    QList<Account> retAccounts;
    for (const auto& [category, account] : data_.value(account_type).asKeyValueRange()) {
        for (const auto& [account_name, _] : account.asKeyValueRange()) {
            retAccounts.push_back(Account(account_type, category, account_name));
        }
    }
    return retAccounts;
}

HouseholdMoney Transaction::getHouseholdMoney(const Account& account) const {
    if (account.type == Account::Equity) {
        qDebug() << Q_FUNC_INFO << "Equity shouldn't be here.";
        return HouseholdMoney();
    }

    if (accountExist(account)) {
        return data_.value(account.type).value(account.category).value(account.name);
    } else {
        return HouseholdMoney();
    }
}

void Transaction::addHouseholdMoney(const Account& account, const HouseholdMoney& household_money) {
    if (account.type == Account::Equity) {
        qDebug() << Q_FUNC_INFO << "Transaction don't store equity, it's calculated by others.";
        return;
    }
    data_[account.type][account.category][account.name] += household_money;
}

HouseholdMoney Transaction::getRetainedEarnings() const {
    HouseholdMoney household_money;
    for (Account::Type account_type : {Account::Revenue, Account::Expense}) {
        for (const auto& [category, accounts] : data_[Account::Revenue].asKeyValueRange()) {
            for (const auto& [account, households] : accounts.asKeyValueRange()) {
                for (const auto& [household, money] : households.asKeyValueRange()) {
                    if (account_type == Account::Revenue) {
                        household_money[household] += money;
                    } else {
                        household_money[household] -= money;
                    }
                    if (household_money[household].isZero()) {
                        household_money.remove(household);
                    }
                }
            }
        }
    }
    return household_money;
}

HouseholdMoney Transaction::getXXXContributedCapital() const {
    return HouseholdMoney(date_time.date(), Currency::USD);
}

//////////////////// Transaction Filter ////////////////////////////
TransactionFilter::TransactionFilter(const QList<Account>& accounts)
    : Transaction(QDateTime(QDate(1990, 05, 25), QTime(0, 0, 0)), "") {
    for (const Account& account : accounts) {
        addAccount(account);
    }
}

TransactionFilter& TransactionFilter::addAccount(const Account& account) {
    data_[account.type][account.category][account.name]["foo"] = Money(QDate(), "$1");
    return *this;
}

TransactionFilter& TransactionFilter::fromTime(const QDateTime& start_time) {
  date_time = start_time;
  return *this;
}

TransactionFilter& TransactionFilter::toTime(const QDateTime& end_time) {
  end_date_time = end_time;
  return *this;
}

TransactionFilter& TransactionFilter::setDescription(const QString& description) {
  this->description = description;
  return *this;
}

TransactionFilter& TransactionFilter::useOr() {
  use_or = true;
  return *this;
}

TransactionFilter& TransactionFilter::useAnd() {
  use_or = false;
  return *this;
}

TransactionFilter& TransactionFilter::orderByAscending() {
  ascending_order = true;
  return *this;
}

TransactionFilter& TransactionFilter::orderByDescending() {
  ascending_order = false;
  return *this;
}

TransactionFilter& TransactionFilter::setLimit(int lim) {
  limit = lim;
  return *this;
}

//////////////////// Financial Summary /////////////////////////////
FinancialStat::FinancialStat() : Transaction() {}

HouseholdMoney FinancialStat::getHouseholdMoney(const Account &account) const {
  if (account == Account(Account::Equity, "Retained Earnings", "Retained Earning")) {
    return retainedEarnings;
  } else if (account == Account(Account::Equity, "Retained Earnings", "Currency Error")) {
    return HouseholdMoney("All", currencyError);
  } else if (account == Account(Account::Equity, "Retained Earnings", "Transaction Error")) {
    return HouseholdMoney("All", transactionError);
  } else if (account == Account(Account::Equity, "Contributed Capitals", "Contributed Capital")) {
    return HouseholdMoney(); // Empty HouseholdMoney.
  }
  return Transaction::getHouseholdMoney(account);
}

QList<Account> FinancialStat::getAccounts() const {
    QList<Account> accounts = Transaction::getAccounts();
    accounts.push_back(Account(Account::Equity, "Retained Earnings", "Retained Earning"));
    accounts.push_back(Account(Account::Equity, "Retained Earnings", "Currency Error"));
    accounts.push_back(Account(Account::Equity, "Retained Earnings", "Transaction Error"));
    accounts.push_back(Account(Account::Equity, "Contributed Capitals", "Contributed Capital"));
    return accounts;
}

void FinancialStat::changeDate(const QDate& nextDate) {
    // Skip when next transaction is the same day of current transaction.
    if (date_time.date() == nextDate) {
        return;
    }
    Money before(date_time.date(), currencyError.currency());
    Money after(nextDate, currencyError.currency());

    for (Account::Type account_type : {Account::Asset, Account::Liability}) {
        for (const auto& [category, accounts] : data_[account_type].asKeyValueRange()) {
            for (const auto& [account, households] : accounts.asKeyValueRange()) {
                for (const auto& [household, money] : households.asKeyValueRange()) {
                    if (account_type == Account::Asset) {
                        before += money;
                        after += money;
                    } else {
                        before -= money;
                        after -= money;
                    }
                }
            }
        }
    }

    currencyError += after - before;
}
