#include "transaction.h"

Transaction::Transaction(const QDateTime& date_time, const QString& description)
  : date_time(date_time), description(description) {
  data_[Account::Expense];
  data_[Account::Revenue];
  data_[Account::Asset];
  data_[Account::Liability];
}

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
  for (const Account &account : getAccounts()) {
    transaction.addMoneyArray(account, this->getMoneyArray(account));

    if (transaction.getMoneyArray(account).isZero()) {
      transaction.data_[account.type][account.category].remove(account.name);
      if (transaction.data_[account.type][account.category].isEmpty()) {
        transaction.data_[account.type].remove(account.category);
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
    for (const Account::Type &tableType : data_.keys()) {
        clear(tableType);
    }
}

void Transaction::clear(Account::Type tableType)
{
    data_[tableType].clear();
}

bool Transaction::accountExist(const Account &account) const {
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
  for (const Account &account : getAccounts()) {
    if (account.type == Account::Expense or account.type == Account::Asset)
      sum += getMoneyArray(account).sum();
    else
      sum -= getMoneyArray(account).sum();
  }

  return sum;
}

QStringList Transaction::validate() const {
    QStringList errorMessage;
    if (description.isEmpty())
        errorMessage << "Description is empty.";
    if (data_.value(Account::Asset).isEmpty() and
        data_.value(Account::Expense).isEmpty() and
        data_.value(Account::Revenue).isEmpty() and
        data_.value(Account::Liability).isEmpty())
        errorMessage << "No account entries.";
    if (qFabs(getCheckSum().amount_) > 0.005)
        errorMessage << "The sum of the transaction is not zero: " + QString::number(getCheckSum().amount_);

    return errorMessage;
}

QString Transaction::toString(Account::Type table_type) const {
    QStringList result;
    for (const Account& account : getAccounts(table_type)) {
        MoneyArray moneyArray = getMoneyArray(account);
        if (!moneyArray.isZero()) {
            result << QString("[%1|%2: %3]").arg(account.category, account.name, moneyArray.toString());
        }
    }
    return result.join("\n");
}

QJsonObject Transaction::toJson() const {
    QJsonObject json;
    for (const auto& [account_type, categories] : data_.asKeyValueRange()) {
        QJsonObject json_accounts;
        for (const auto& [category_name, accounts] : categories.asKeyValueRange()) {
            for (const auto& [account_name, money_array] : accounts.asKeyValueRange()) {
                if (!money_array.isZero()) {
                    json_accounts[category_name + "|" + account_name] = money_array.toString();
                }
            }
        }
        if (!json_accounts.isEmpty()) {
            json[Account::kTableName.value(account_type)] = json_accounts;
        }
    }
    return json;
}

void Transaction::setData(const QJsonObject& json) {
    for (auto account_type : {Account::Asset, Account::Liability, Account::Expense, Account::Revenue}) {
        QJsonObject categories = json[Account::kTableName.value(account_type)].toObject();
        for (const QString& account : categories.keys()) {
            QString category_name  = account.split("|").at(0);
            QString account_name   = account.split("|").at(1);
            QString amounts = categories.value(account).toString();
            addMoneyArray(Account(account_type, category_name, account_name), MoneyArray(date_time.date(), amounts));
        }
    }
}

QList<Account> Transaction::getAccounts() const {
  QList<Account> all_ccounts;
  for (const Account::Type& tableType : {Account::Asset, Account::Expense, Account::Revenue, Account::Liability}) {
    for (const QString& category : data_.value(tableType).keys()) {
      for (const QString& name : data_.value(tableType).value(category).keys()) {
        all_ccounts.push_back(Account(tableType, category, name));
      }
    }
  }
  return all_ccounts;
}

QList<Account> Transaction::getAccounts(Account::Type account_type) const {
    QList<Account> retAccounts;
    for (const QString& category : data_.value(account_type).keys()) {
        for (const QString& name : data_.value(account_type).value(category).keys()) {
            retAccounts.push_back(Account(account_type, category, name));
        }
    }
    return retAccounts;
}

MoneyArray Transaction::getMoneyArray(const Account &account) const {
    if (account.type == Account::Equity) {
        qDebug() << Q_FUNC_INFO << "Equity should be here.";
    }

    if (accountExist(account)) {
        return data_.value(account.type).value(account.category).value(account.name);
    } else {
        return MoneyArray(date_time.date(), Currency::USD);
    }
}

void Transaction::addMoneyArray(const Account& account, const MoneyArray& moneyArray) {
    if (account.type == Account::Equity) {
        qDebug() << Q_FUNC_INFO << "Transaction don't store equity, it's calculated by others.";
        return;
    }

    if (!accountExist(account)) {
        data_[account.type][account.category][account.name] = MoneyArray(moneyArray.date_, moneyArray.currency());
    }
    data_[account.type][account.category][account.name] += moneyArray;
}

MoneyArray Transaction::getRetainedEarnings() const {
  MoneyArray ret(date_time.date(), Currency::USD);
  for (const Account &account : getAccounts(Account::Revenue)) {
    ret += getMoneyArray(account);
  }
  for (const Account &account : getAccounts(Account::Expense)) {
    ret -= getMoneyArray(account);
  }
  return ret;
}

MoneyArray Transaction::getXXXContributedCapital() const
{
    return MoneyArray(date_time.date(), Currency::USD);
}

//////////////////// Transaction Filter ////////////////////////////
TransactionFilter::TransactionFilter(const QList<Account>& accounts)
  : Transaction(QDateTime(QDate(1990, 05, 25), QTime(0, 0, 0)), "") {
  for (const Account& account : accounts) {
    addAccount(account);
  }
}

TransactionFilter& TransactionFilter::addAccount(const Account& account) {
  addMoneyArray(account, MoneyArray(QDate(), "$1"));
  return *this;
}

TransactionFilter& TransactionFilter::fromTime(const QDateTime& start_time) {
  date_time = start_time;
  return *this;
}

TransactionFilter& TransactionFilter::toTime(const QDateTime& end_time) {
  end_date_time_ = end_time;
  return *this;
}

TransactionFilter& TransactionFilter::setDescription(const QString& description) {
  this->description = description;
  return *this;
}

TransactionFilter& TransactionFilter::useOr() {
  use_or_ = true;
  return *this;
}

TransactionFilter& TransactionFilter::useAnd() {
  use_or_ = false;
  return *this;
}

TransactionFilter& TransactionFilter::orderByAscending() {
  ascending_order_ = true;
  return *this;
}

TransactionFilter& TransactionFilter::orderByDescending() {
  ascending_order_ = false;
  return *this;
}

TransactionFilter& TransactionFilter::setLimit(int limit) {
  limit_ = limit;
  return *this;
}

//////////////////// Financial Summary /////////////////////////////
FinancialStat::FinancialStat() : Transaction() {}

MoneyArray FinancialStat::getMoneyArray(const Account &account) const {
  if (account == Account(Account::Equity, "Retained Earnings", "Retained Earning")) {
    return retainedEarnings;
  } else if (account == Account(Account::Equity, "Retained Earnings", "Currency Error")) {
    return currencyError;
  } else if (account == Account(Account::Equity, "Retained Earnings", "Transaction Error")) {
    return transactionError;
  } else if (account == Account(Account::Equity, "Contributed Capitals", "Contributed Capital")) {
    return MoneyArray(date_time.date(), Currency::USD); // Empty money array.
  }
  return Transaction::getMoneyArray(account);
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
  MoneyArray before(date_time.date(), currencyError.currency());
  MoneyArray after(nextDate, currencyError.currency());
  for (const Account& account : Transaction::getAccounts(Account::Asset)) {
    before += getMoneyArray(account);
    after += getMoneyArray(account);
  }
  for (const Account& account : Transaction::getAccounts(Account::Liability)) {
    before -= getMoneyArray(account);
    after -= getMoneyArray(account);
  }

  currencyError += after - before;
}
