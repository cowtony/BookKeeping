#include "Transaction.h"

Transaction::Transaction()
{
    (*this)[Account::Expense];
    (*this)[Account::Asset];
    (*this)[Account::Revenue];
    (*this)[Account::Liability];
//    m_description.clear();
//    m_dateTime = QDateTime();
}

Transaction Transaction::operator +(Transaction transaction) const
{
    // dateTime is the maximum dateTime
    transaction.m_dateTime = transaction.m_dateTime > m_dateTime? transaction.m_dateTime : m_dateTime;

    // merge description
    if (m_description.contains(transaction.m_description))
        transaction.m_description = m_description;
    else if (!transaction.m_description.contains(m_description))
        transaction.m_description = m_description + "; " + transaction.m_description;

    // Add up account
    for (const Account &account : getAccounts())
    {
        transaction.addMoneyArray(account, this->getMoneyArray(account));

        if (transaction.getMoneyArray(account).isZero())
        {
            transaction[account.m_table][account.m_category].remove(account.m_name);
            if (transaction[account.m_table][account.m_category].isEmpty())
                transaction[account.m_table].remove(account.m_category);
        }
    }

    return transaction;
}

void Transaction::operator +=(const Transaction &t)
{
    *this = *this + t;
}

void Transaction::clear() {
    m_dateTime = QDateTime();
    m_description.clear();
    for (const Account::TableType &tableType : keys()) {
        clear(tableType);
    }
}

void Transaction::clear(const Account::TableType &tableType)
{
    (*this)[tableType].clear();
}

bool Transaction::accountExist(const Account &account) const
{
    if (contains(account.m_table))
        if (value(account.m_table).contains(account.m_category))
            if (value(account.m_table).value(account.m_category).contains(account.m_name))
                return true;
    return false;
}

Money Transaction::getCheckSum() const {
  Money sum(m_dateTime.date());
  for (const Account &account : getAccounts()) {
    if (account.m_table == Account::Expense or account.m_table == Account::Asset)
      sum += getMoneyArray(account).sum();
    else
      sum -= getMoneyArray(account).sum();
  }

  return sum;
}

QStringList Transaction::validation() const {
    QStringList errorMessage;
    if (m_description.isEmpty())
        errorMessage << "Description is empty.";
    if (value(Account::Asset).isEmpty() and
        value(Account::Expense).isEmpty() and
        value(Account::Revenue).isEmpty() and
        value(Account::Liability).isEmpty())
        errorMessage << "No account entries.";
    if (qFabs(getCheckSum().m_amount) > 0.005)
        errorMessage << "The sum of the transaction is not zero: " + QString::number(getCheckSum().m_amount);

    return errorMessage;
}

QString Transaction::dataToString(const Account::TableType &tableType) const {
    QStringList retString;
    for (const Account &account : getAccounts(tableType))
    {
        MoneyArray moneyArray = getMoneyArray(account);    
        if (!moneyArray.isZero())
            retString << "[" + account.m_category + "|" + account.m_name + ": " + moneyArray.toString() + "]";
    }

    return retString.join("; ");
}

void Transaction::stringToData(const Account::TableType &tableType, const QString &data) {
    if (data == "Empty" or data.isEmpty())
        return;

    for (QString accountInfo: data.split("; "))      // [Category|AccountName1: $1.23, $2.34]; [Category2|AccountName2: ¥3.21, ¥2.31]
    {
        accountInfo = accountInfo.mid(1, accountInfo.length() - 2); // Remove '[' and ']'

        QString cateNname = accountInfo.split(": ").at(0);
        QString amounts   = accountInfo.split(": ").at(1);
        QString category  = cateNname.split("|").at(0);
        QString name      = cateNname.split("|").at(1);

        Account account(tableType, category, name);
        if (accountExist(account))
            qDebug() << Q_FUNC_INFO << "Transaction has duplicated account" << m_dateTime;
        addMoneyArray(account, MoneyArray(m_dateTime.date(), amounts));
    }
}

QList<Account> Transaction::getAccounts() const {
    QList<Account> retAccounts;
    for (const Account::TableType &tableType : keys()) {
        for (const QString &category : value(tableType).keys()) {
            for (const QString &name : value(tableType).value(category).keys()) {
                retAccounts.push_back(Account(tableType, category, name));
            }
        }
    }
    return retAccounts;
}

QList<Account> Transaction::getAccounts(const Account::TableType &tableType) const
{
    QList<Account> retAccounts;
    for (const QString &category : value(tableType).keys()) {
        for (const QString &name : value(tableType).value(category).keys()) {
            retAccounts.push_back(Account(tableType, category, name));
        }
    }
    return retAccounts;
}

MoneyArray Transaction::getMoneyArray(const Account &account) const {
    if (account.m_table == Account::Equity)
        qDebug() << Q_FUNC_INFO << "Equity should be here.";

    if (accountExist(account))
        return value(account.m_table).value(account.m_category).value(account.m_name);
    else
        return MoneyArray(m_dateTime.date(), USD);
}

void Transaction::addMoneyArray(const Account &account, const MoneyArray &moneyArray) {
  if (account.m_table == Account::Equity) {
    qDebug() << Q_FUNC_INFO << "Transaction don't store equity, it's calculated by others.";
    return;
  }

  if (!accountExist(account))
    (*this)[account.m_table][account.m_category][account.m_name] = MoneyArray(moneyArray.m_date, moneyArray.m_currency);

  (*this)[account.m_table][account.m_category][account.m_name] += moneyArray;
}

MoneyArray Transaction::getRetainedEarnings() const
{
    MoneyArray ret(m_dateTime.date(), USD);
    for (const Account &account : getAccounts(Account::Revenue))
        ret += getMoneyArray(account);

    for (const Account &account : getAccounts(Account::Expense))
        ret -= getMoneyArray(account);

    return ret;
}

MoneyArray Transaction::getXXXContributedCapital() const
{
    return MoneyArray(m_dateTime.date(), USD);
}

//////////////////// Financial Summary /////////////////////////////
MoneyArray FinancialStat::getMoneyArray(const Account &account) const {
    if (account == Account(Account::Equity, "Retained Earnings", "Retained Earnings"))
        return retainedEarnings;
    else if (account == Account(Account::Equity, "Contributed Capital", "Contributed Capital"))
        return MoneyArray(m_dateTime.date(), USD);

    return Transaction::getMoneyArray(account);
}

QList<Account> FinancialStat::getAccounts() const {
  QList<Account> accounts = Transaction::getAccounts();
  accounts.push_back(Account(Account::Equity, "Retained Earnings", "Retained Earnings"));
  accounts.push_back(Account(Account::Equity, "Contributed Capital", "Contributed Capital"));
  return accounts;
}
