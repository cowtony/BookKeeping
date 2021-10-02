#include "Transaction.h"

Transaction::Transaction(const QDateTime& date_time, const QString& description)
  : date_time_(date_time), description_(description) {
  data_[Account::Expense];
  data_[Account::Revenue];
  data_[Account::Asset];
  data_[Account::Liability];
}

Transaction Transaction::operator +(Transaction transaction) const {
  // dateTime is the maximum dateTime
  transaction.date_time_ = transaction.date_time_ > date_time_? transaction.date_time_ : date_time_;

  // merge description
  if (description_.contains(transaction.description_)) {
    transaction.description_ = description_;
  } else if (!transaction.description_.contains(description_)) {
    transaction.description_ = description_ + "; " + transaction.description_;
  }

  // Add up account
  for (const Account &account : getAccounts()) {
    transaction.addMoneyArray(account, this->getMoneyArray(account));

    if (transaction.getMoneyArray(account).isZero()) {
      transaction.data_[account.table_][account.category_].remove(account.name_);
      if (transaction.data_[account.table_][account.category_].isEmpty()) {
        transaction.data_[account.table_].remove(account.category_);
      }
    }
  }

  return transaction;
}

void Transaction::operator +=(const Transaction &t) {
  *this = *this + t;
}

void Transaction::clear() {
    date_time_ = QDateTime();
    description_.clear();
    for (const Account::TableType &tableType : data_.keys()) {
        clear(tableType);
    }
}

void Transaction::clear(Account::TableType tableType)
{
    data_[tableType].clear();
}

bool Transaction::accountExist(const Account &account) const {
  if (data_.contains(account.table_)) {
    if (data_.value(account.table_).contains(account.category_)) {
      if (data_.value(account.table_).value(account.category_).contains(account.name_)) {
        return true;
      }
    }
  }
  return false;
}

Money Transaction::getCheckSum() const {
  Money sum(date_time_.date());
  for (const Account &account : getAccounts()) {
    if (account.table_ == Account::Expense or account.table_ == Account::Asset)
      sum += getMoneyArray(account).sum();
    else
      sum -= getMoneyArray(account).sum();
  }

  return sum;
}

QStringList Transaction::validation() const {
    QStringList errorMessage;
    if (description_.isEmpty())
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

QString Transaction::dataToString(Account::TableType tableType) const {
  QStringList retString;
  for (const Account& account : getAccounts(tableType)) {
    MoneyArray moneyArray = getMoneyArray(account);
    if (!moneyArray.isZero()) {
      retString << "[" + account.category_ + "|" + account.name_ + ": " + moneyArray.toString() + "]";
    }
  }
  return retString.join("; ");
}

void Transaction::stringToData(Account::TableType tableType, const QString& data) {
  if (data == "Empty" or data.isEmpty()) {
    return;
  }

  // [Category|AccountName1: $1.23, $2.34]; [Category2|AccountName2: ¥3.21, ¥2.31]
  for (QString accountInfo: data.split("; ")) {
    accountInfo = accountInfo.mid(1, accountInfo.length() - 2); // Remove '[' and ']'

    QString cateNname = accountInfo.split(": ").at(0);
    QString amounts   = accountInfo.split(": ").at(1);
    QString category  = cateNname.split("|").at(0);
    QString name      = cateNname.split("|").at(1);

    Account account(tableType, category, name);
    if (accountExist(account)) {
      qDebug() << Q_FUNC_INFO << "Transaction has duplicated account" << date_time_;
    }
    addMoneyArray(account, MoneyArray(date_time_.date(), amounts));
  }
}

QList<Account> Transaction::getAccounts() const {
  QList<Account> all_ccounts;
  for (const Account::TableType& tableType : {Account::Asset, Account::Expense, Account::Revenue, Account::Liability}) {
    for (const QString& category : data_.value(tableType).keys()) {
      for (const QString& name : data_.value(tableType).value(category).keys()) {
        all_ccounts.push_back(Account(tableType, category, name));
      }
    }
  }
  return all_ccounts;
}

QList<Account> Transaction::getAccounts(Account::TableType tableType) const
{
    QList<Account> retAccounts;
    for (const QString &category : data_.value(tableType).keys()) {
        for (const QString &name : data_.value(tableType).value(category).keys()) {
            retAccounts.push_back(Account(tableType, category, name));
        }
    }
    return retAccounts;
}

MoneyArray Transaction::getMoneyArray(const Account &account) const {
    if (account.table_ == Account::Equity)
        qDebug() << Q_FUNC_INFO << "Equity should be here.";

    if (accountExist(account))
        return data_.value(account.table_).value(account.category_).value(account.name_);
    else
        return MoneyArray(date_time_.date(), USD);
}

void Transaction::addMoneyArray(const Account& account, const MoneyArray& moneyArray) {
  if (account.table_ == Account::Equity) {
    qDebug() << Q_FUNC_INFO << "Transaction don't store equity, it's calculated by others.";
    return;
  }

  if (!accountExist(account)) {
    data_[account.table_][account.category_][account.name_] = MoneyArray(moneyArray.date_, moneyArray.currency());
  }
  data_[account.table_][account.category_][account.name_] += moneyArray;
}

MoneyArray Transaction::getRetainedEarnings() const {
  MoneyArray ret(date_time_.date(), USD);
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
    return MoneyArray(date_time_.date(), USD);
}

//////////////////// Transaction Filter ////////////////////////////
TransactionFilter::TransactionFilter(const QDateTime& start_time,
                                     const QDateTime& end_time,
                                     const QString& description,
                                     const QList<Account>& accounts,
                                     bool use_union,
                                     bool ascending_order)
  : Transaction(start_time, description), end_date_time_(end_time), use_union_(use_union), ascending_order_(ascending_order) {
  for (const Account& account : accounts) {
    addAccount(account);
  }
}

void TransactionFilter::addAccount(const Account& account) {
  addMoneyArray(account, MoneyArray(QDate(), "$1"));
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
    return MoneyArray(date_time_.date(), USD); // Empty money array.
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
  if (date_time_.date() == nextDate) {
    return;
  }
  MoneyArray before(date_time_.date(), currencyError.currency());
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
