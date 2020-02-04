#include "Money.h"
#include <QtMath>
#include <QLocale>
#include "Currency.h"

/****************** Money ****************************/
Money::Money(const QDate& date, Currency_e currency, double amount)
  : m_date(date), m_amount(amount), m_currency(currency) {
}

Money::Money(const QDate& date, QString money_str, Currency_e currency)
  : m_date(date), m_amount(0.00), m_currency(currency) {
  if (money_str.isEmpty()) {
    return;
  }
  if (money_str.front() == '(' and money_str.back() == ')')
        money_str = "-" + money_str.mid(1, money_str.length() - 2);

  int sign = 1;
  if (money_str.front() == '-') {
    sign = -1;
    money_str.remove(0, 1);
  }

  if (Currency::Symbol_1.values().contains(money_str.left(1))) {
    m_currency = Currency::Symbol_1.key(money_str.left(1));
    money_str.remove(0, 1);
  } else if (Currency::Symbol_3.values().contains(money_str.left(3))) {
    m_currency = Currency::Symbol_3.key(money_str.left(3));
    money_str.remove(0, 3);
  }

  bool ok;
  m_amount = sign * QLocale(QLocale::English).toDouble(money_str, &ok);

  if (!ok) {
    qDebug() << Q_FUNC_INFO << money_str;
  }
}

Currency_e Money::currency() const {
  return m_currency;
}

double Money::getRoundedAmount() const
{
    return qRound(m_amount * 100) / 100.0;
}

Money Money::operator -() const {
    return Money(m_date, m_currency, -m_amount);
}

Money Money::operator /(int val) const
{
    if (val == 0)
        qDebug() << Q_FUNC_INFO << val;
    return Money(m_date, m_currency, m_amount / val);
}

Money Money::operator +(Money money) const
{
    money.m_date = m_date > money.m_date? m_date : money.m_date;
    money.changeCurrency(m_currency);
    money.m_amount = m_amount + money.m_amount;
    return money;
}

Money Money::operator -(Money money) const {
  money.m_date = m_date > money.m_date? m_date : money.m_date;
  money.changeCurrency(m_currency);
  money.m_amount = m_amount - money.m_amount;
  return money;
}

Money Money::operator *(double rateOfReturn) const {
  Money money = *this;
  money.m_amount *= rateOfReturn;
  return money;
}

bool Money::operator <(Money money) const {
  money.changeCurrency(m_currency);
  return m_amount < money.m_amount;
}

void Money::operator +=(const Money &money)
{
    *this = *this + money;
}

void Money::operator -=(const Money &money)
{
    *this = *this - money;
}

void Money::changeCurrency(Currency_e currency_e) {
  m_amount  *= g_currency.getCurrencyRate(m_date, m_currency, currency_e);
  m_currency = currency_e;
}

QString Money::toString() const {
    return QLocale(QLocale::English).toCurrencyString(getRoundedAmount(), Currency::Symbol_1.value(m_currency), 2);
}

Money Money::round() const
{
    Money money = *this;
    money.m_amount = getRoundedAmount();
    return money;
}

/*************** Money Array ********************/
MoneyArray::MoneyArray(const QDate& date, Currency_e currency)
  : Money(date, currency, 0.00) {
  m_amounts.clear();
}

MoneyArray::MoneyArray(const QDate& date, const QString& money_str)
  : Money(date, USD, 0.00) {
  for (const QString &moneyString : money_str.split(", ")) {
    Money money(m_date, moneyString);
    changeCurrency(money.currency());
    push_back(money);
  }
}

MoneyArray::MoneyArray(const Money& money)
  : Money(money.m_date, money.currency(), 0.00) {
  m_amounts.push_back(money.m_amount);
}

MoneyArray MoneyArray::operator -() const {
  MoneyArray moneyArray(*this);
  for (int i = 0; i < m_amounts.size(); i++) {
    moneyArray.m_amounts[i] = -moneyArray.m_amounts.at(i);
  }
  return moneyArray;
}

bool MoneyArray::isZero() const
{
    for (const double &amount : m_amounts)
        if (qFabs(amount) > 1e-4)
            return false;
    return true;
}

Money MoneyArray::sum() const {
  Money money(m_date, m_currency, 0);
  for (const double &amount : m_amounts)
    money.m_amount += amount;
  return money;
}

Money MoneyArray::getMoney(int index) const
{
    Money money(m_date, m_currency, 0);
    if (index < m_amounts.size())
        money.m_amount = m_amounts.at(index);
    return money;
}

void MoneyArray::push_back(Money money) {
  if (m_amounts.isEmpty()) {
    m_date = money.m_date;
  }
  money.changeCurrency(m_currency);
  m_amounts.push_back(money.m_amount);
}

QString MoneyArray::toString() const
{
    QStringList result;
    for (const double &amount : m_amounts)
    {
        result << Money(m_date, m_currency, amount).toString();
    }
    return result.join(", ");
}

void MoneyArray::changeCurrency(Currency_e currency) {
  double currencyRate = g_currency.getCurrencyRate(m_date, m_currency, currency);
  m_currency = currency;
  for (double& amount : m_amounts) {
    amount *= currencyRate;
  }
}

MoneyArray MoneyArray::operator +(const MoneyArray &moneyArray) const
{
    return addMinus(moneyArray, [](double x, double y){return x + y;});
}

void MoneyArray::operator +=(const MoneyArray &moneyArray)
{
    *this = *this + moneyArray;
}

MoneyArray MoneyArray::operator -(const MoneyArray &moneyArray) const
{
    return addMinus(moneyArray, [](double x, double y){return x - y;});
}

void MoneyArray::operator -=(const MoneyArray &moneyArray)
{
    *this = *this - moneyArray;
}

MoneyArray MoneyArray::addMinus(MoneyArray moneyArray, double f(double, double)) const
{
    moneyArray.m_date = m_date > moneyArray.m_date? m_date : moneyArray.m_date;

    moneyArray.changeCurrency(m_currency);

    while (moneyArray.m_amounts.size() < m_amounts.size())
        moneyArray.m_amounts.push_back(0.00);

    for (int i = 0; i < moneyArray.m_amounts.size(); i++)
    {
        if (m_amounts.size() <= i)
            moneyArray.m_amounts[i] = f(0              , moneyArray.m_amounts.at(i));
        else
            moneyArray.m_amounts[i] = f(m_amounts.at(i), moneyArray.m_amounts.at(i));
    }

    while (!moneyArray.m_amounts.isEmpty() and moneyArray.m_amounts.back() == 0.0)
        moneyArray.m_amounts.pop_back();

    return moneyArray;
}
