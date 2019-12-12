#include "Money.h"
#include <QtMath>
#include <QLocale>
#include "Currency.h"

/****************** Money ****************************/
Money::Money(const QDate &date, const Currency_e& currency, const double& amount)
    : m_date(date), m_amount(amount), m_currency(currency)
{
}

Money::Money(const QDate &date, QString p_money_s, const Currency_e &currency)
    : m_date(date), m_amount(0.00), m_currency(currency)
{
    if (p_money_s.isEmpty())
        return;

    if (p_money_s.front() == '(' and p_money_s.back() == ')')
        p_money_s = "-" + p_money_s.mid(1, p_money_s.length() - 2);

    int sign = 1;
    if (p_money_s.front() == '-')
    {
        sign = -1;
        p_money_s.remove(0, 1);
    }

    if (Currency::Symbol_1.values().contains(p_money_s.left(1)))
    {
        m_currency = Currency::Symbol_1.key(p_money_s.left(1));
        p_money_s.remove(0, 1);
    }
    else if (Currency::Symbol_3.values().contains(p_money_s.left(3)))
    {
        m_currency = Currency::Symbol_3.key(p_money_s.left(3));
        p_money_s.remove(0, 3);
    }

    bool ok;
    m_amount = sign * QLocale(QLocale::English).toDouble(p_money_s, &ok);

    if (!ok)
        qDebug() << Q_FUNC_INFO << p_money_s;
}

Currency_e Money::getCurrency() const
{
    return m_currency;
}

double Money::getRoundedAmount() const
{
    return qRound(m_amount * 100) / 100.0;
}

Money Money::operator -() const {
    return Money(m_date, m_currency, -m_amount);
}

Money Money::operator /(const int &val) const
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

Money Money::operator -(Money money) const
{
    money.m_date = m_date > money.m_date? m_date : money.m_date;
    money.changeCurrency(m_currency);
    money.m_amount = m_amount - money.m_amount;
    return money;
}

void Money::operator +=(const Money &money)
{
    *this = *this + money;
}

void Money::operator -=(const Money &money)
{
    *this = *this - money;
}

void Money::changeCurrency(const Currency_e &currency_e)
{
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
MoneyArray::MoneyArray()
{

}

MoneyArray::MoneyArray(const QDate &date, const Currency_e &currency)
    : m_currency(currency), m_date(date)
{
    m_amounts.clear();
}

MoneyArray::MoneyArray(const QDate &date, const QString &p_money_s)
    : m_currency(USD), m_date(date)
{
    for (const QString &moneyString : p_money_s.split(", "))
    {
        Money money(m_date, moneyString, m_currency);
        m_currency = money.getCurrency();
        push_back(money);
    }
}

MoneyArray MoneyArray::operator -() const
{
    MoneyArray moneyArray(*this);
    for (int i = 0; i < m_amounts.size(); i++)
        moneyArray.m_amounts[i] = -moneyArray.m_amounts.at(i);
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

Money MoneyArray::getMoney(const int &index) const
{
    Money money(m_date, m_currency, 0);
    if (index < m_amounts.size())
        money.m_amount = m_amounts.at(index);
    return money;
}

void MoneyArray::push_back(Money money) {
  if (m_amounts.size() == 0)
  {
     m_currency = money.getCurrency();
     m_date = money.m_date;
  } else
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

void MoneyArray::changeCurrency(const Currency_e &currency)
{
    double currencyRate = g_currency.getCurrencyRate(m_date, m_currency, currency);
    for (double &amount : m_amounts)
    {
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
