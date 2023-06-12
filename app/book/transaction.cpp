#include "transaction.h"

Transaction::Transaction(const QDateTime& date_time, const QString& description)
    : date_time(date_time),
      description(description),
      id(-1),
      data_({{Account::Expense, {}}, {Account::Expense, {}}, {Account::Expense, {}}, {Account::Expense, {}}}) {}

Transaction Transaction::operator +(Transaction transaction) const {
    // dateTime is the maximum dateTime
    transaction.date_time = qMax(transaction.date_time, date_time);

    // merge description
    if (description.contains(transaction.description)) {
        transaction.description = description;
    } else if (!transaction.description.contains(description)) {
        transaction.description = description + "; " + transaction.description;
    }

    // Add up account
    for (const auto& [account, household_money] : getAccounts()) {
        for (const auto& [household, money] : household_money.asKeyValueRange()) {
            transaction.addMoney(account, household, money);
            // Recursively remove empty key.
            if (transaction.data_[account->accountType()][account->categoryName()][account->accountName()].second[household].isZero()) {
                transaction.data_[account->accountType()][account->categoryName()][account->accountName()].second.remove(household);
                if (transaction.data_[account->accountType()][account->categoryName()][account->accountName()].second.isEmpty()) {
                    transaction.data_[account->accountType()][account->categoryName()].remove(account->accountName());
                    if (transaction.data_[account->accountType()][account->categoryName()].isEmpty()) {
                        transaction.data_[account->accountType()].remove(account->categoryName());
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

Money Transaction::getCheckSum() const {
    Money sum(date_time.date());
    for (const auto& [account_type, categories] : data_.asKeyValueRange()) {
        for (const auto& [category, accounts] : categories.asKeyValueRange()) {
            for (const auto& [account, data_pair] : accounts.asKeyValueRange()) {
                const auto& [account_id, households] = data_pair;
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
        for (const auto& [account, data_pair] : accounts.asKeyValueRange()) {
            const auto& [account_id, household_money] = data_pair;
            for (const auto& [household, money] : household_money.asKeyValueRange()) {
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

bool Transaction::contains(const Account& account) const {
    if (data_.contains(account.accountType())) {
        const auto& categories = data_.value(account.accountType());
        if (categories.contains(account.categoryName())) {
            const auto& accounts = categories.value(account.categoryName());
            if (accounts.contains(account.accountName())) {
                return !accounts.value(account.accountName()).first.isNull();
            }
        }
    }
    return false;
}

QList<QPair<QSharedPointer<Account>, HouseholdMoney>> Transaction::getAccounts() const {
    QList<QPair<QSharedPointer<Account>, HouseholdMoney>> result;
    for (const Account::Type& account_type : {Account::Asset, Account::Expense, Account::Revenue, Account::Liability}) {
        result << getAccounts(account_type);
    }
    return result;
}

QList<QPair<QSharedPointer<Account>, HouseholdMoney>> Transaction::getAccounts(Account::Type account_type) const {
    QList<QPair<QSharedPointer<Account>, HouseholdMoney>> result;
    for (const auto& [category_name, accounts] : data_.value(account_type).asKeyValueRange()) {
        for (const auto& [account_name, data_pair] : accounts.asKeyValueRange()) {
            result << data_pair;
        }
    }
    return result;
}

HouseholdMoney Transaction::getHouseholdMoney(const Account& account) const {
    return getHouseholdMoney(account.accountType(), account.categoryName(), account.accountName());
}

HouseholdMoney Transaction::getHouseholdMoney(Account::Type account_type, const QString &category_name, const QString &account_name) const {
    if (account_type == Account::Equity) {
        qDebug() << Q_FUNC_INFO << "Equity shouldn't be here.";
        return HouseholdMoney();
    }

    if (data_.contains(account_type)) {
        const auto& categories = data_.value(account_type);
        if (categories.contains(category_name)) {
            const auto& accounts = categories.value(category_name);
            if (accounts.contains(account_name)) {
                return accounts.value(account_name).second;
            }
        }
    }
    return HouseholdMoney();
}

void Transaction::addMoney(QSharedPointer<Account> account, const QString &household, Money money) {
    if (!contains(*account)) {
        data_[account->accountType()][account->categoryName()][account->accountName()].first = account;
    }
    data_[account->accountType()][account->categoryName()][account->accountName()].second[household] += money;
}

HouseholdMoney Transaction::getRetainedEarnings() const {
    HouseholdMoney retained_earning;
    for (Account::Type account_type : {Account::Revenue, Account::Expense}) {
        for (const auto& [category_name, accounts] : data_[Account::Revenue].asKeyValueRange()) {
            for (const auto& [account_name, data_pair] : accounts.asKeyValueRange()) {
                const auto& [account, household_money] = data_pair;
                for (const auto& [household, money] : household_money.asKeyValueRange()) {
                    if (account_type == Account::Revenue) {
                        retained_earning[household] += money;
                    } else {
                        retained_earning[household] -= money;
                    }
                    if (retained_earning[household].isZero()) {
                        retained_earning.remove(household);
                    }
                }
            }
        }
    }
    return retained_earning;
}

HouseholdMoney Transaction::getXXXContributedCapital() const {
    return HouseholdMoney(date_time.date(), Currency::USD);
}

//////////////////// Transaction Filter ////////////////////////////
TransactionFilter::TransactionFilter(const QList<QSharedPointer<Account>>& accounts)
    : Transaction(QDateTime(QDate(1990, 05, 25), QTime(0, 0, 0)), "") {
    for (QSharedPointer<Account> account : accounts) {
        addAccount(account);
    }
}

TransactionFilter& TransactionFilter::addAccount(QSharedPointer<Account> account) {
    addMoney(account, "foo", Money(QDate(), "$1"));
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

HouseholdMoney FinancialStat::getHouseholdMoney(Account::Type account_type, const QString& category_name, const QString& account_name) const {
    if (account_type == Account::Equity && category_name == "Retained Earnings" && account_name == "Retained Earning") {
        return retained_earnings;
    } else if (account_type == Account::Equity && category_name == "Retained Earnings" && account_name == "Currency Error") {
        return HouseholdMoney("All", currency_error_);
    } else if (account_type == Account::Equity && category_name == "Retained Earnings" && account_name == "Transaction Error") {
        return HouseholdMoney("All", transaction_error);
    } else if (account_type == Account::Equity && category_name == "Contributed Capitals" && account_name == "Contributed Capital") {
        return HouseholdMoney(); // Empty HouseholdMoney.
    }
    return Transaction::getHouseholdMoney(account_type, category_name, account_name);
}

QList<QPair<QSharedPointer<Account>, HouseholdMoney>> FinancialStat::getAccounts() const {
    QList<QPair<QSharedPointer<Account>, HouseholdMoney>> result = Transaction::getAccounts();
    // TODO: Make sure these are reserved account.
    result << qMakePair(Account::create(-1, -1, Account::Equity, "Retained Earnings", "Retained Earning"),       HouseholdMoney());
    result << qMakePair(Account::create(-1, -1, Account::Equity, "Retained Earnings", "Currency Error"),         HouseholdMoney());
    result << qMakePair(Account::create(-1, -1, Account::Equity, "Retained Earnings", "Transaction Error"),      HouseholdMoney());
    result << qMakePair(Account::create(-1, -1, Account::Equity, "Contributed Capitals", "Contributed Capital"), HouseholdMoney());
    return result;
}

void FinancialStat::changeDate(const QDate& nextDate) {
    // Skip when next transaction is the same day of current transaction.
    if (date_time.date() == nextDate) {
        return;
    }
    Money before(date_time.date(), currency_error_.currency());
    Money after(nextDate, currency_error_.currency());

    for (Account::Type account_type : {Account::Asset, Account::Liability}) {
        for (const auto& [acount_ptr, household_money] : Transaction::getAccounts(account_type)) {
            for (const auto& [household, money] : household_money.asKeyValueRange()) {
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

    currency_error_ += after - before;
}
