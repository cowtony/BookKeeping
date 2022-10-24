#include "money.h"
#include <QtMath>
#include <QLocale>
#include "currency.h"

/****************** Money ****************************/
Money::Money(const QDate& date, Currency::Type currency, double amount)
  : date_(date), amount_(amount), currency_(currency) {
}

Money::Money(const QDate& date, QString money_str, Currency::Type currency)
  : date_(date), amount_(0.00), currency_(currency) {
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

  if (Currency::kCurrencyToSymbol.values().contains(money_str.left(1))) {
    currency_ = Currency::kCurrencyToSymbol.key(money_str.left(1));
    money_str.remove(0, 1);
  } else if (Currency::kCurrencyToCode.values().contains(money_str.left(3))) {
    currency_ = Currency::kCurrencyToCode.key(money_str.left(3));
    money_str.remove(0, 3);
  }

  bool ok;
  amount_ = sign * QLocale(QLocale::English).toDouble(money_str, &ok);

  if (!ok) {
    qDebug() << Q_FUNC_INFO << money_str;
  }
}

Currency::Type Money::currency() const {
  return currency_;
}

double Money::getRoundedAmount() const
{
    return qRound(amount_ * 100) / 100.0;
}

Money Money::operator -() const {
    return Money(date_, currency_, -amount_);
}

Money Money::operator /(int val) const
{
    if (val == 0)
        qDebug() << Q_FUNC_INFO << val;
    return Money(date_, currency_, amount_ / val);
}

Money Money::operator +(Money money) const {
  money.date_ = date_ > money.date_? date_ : money.date_;
  money.changeCurrency(currency_);
  money.amount_ += amount_;
  return money;
}

Money Money::operator -(Money money) const {
  money.date_ = date_ > money.date_? date_ : money.date_;
  money.changeCurrency(currency_);
  money.amount_ = amount_ - money.amount_;
  return money;
}

Money Money::operator *(double rateOfReturn) const {
  Money money = *this;
  money.amount_ *= rateOfReturn;
  return money;
}

bool Money::operator <(Money money) const {
  money.changeCurrency(currency_);
  return amount_ < money.amount_;
}

void Money::operator +=(const Money &money) {
  *this = *this + money;
}

void Money::operator -=(const Money &money)
{
    *this = *this - money;
}

Money& Money::changeCurrency(Currency::Type currency_e) {
  amount_  *= g_currency.getExchangeRate(date_, currency_, currency_e);
  currency_ = currency_e;
  return *this;
}

QString Money::toString() const {
  // TODO: This returns ($100.00) instead of -$100.00 now.
  return QLocale(QLocale::English).toCurrencyString(getRoundedAmount(), Currency::kCurrencyToSymbol.value(currency_), 2);
}

Money Money::round() const
{
    Money money = *this;
    money.amount_ = getRoundedAmount();
    return money;
}

/*************** Money Array ********************/
MoneyArray::MoneyArray(const QDate& date, Currency::Type currency)
  : Money(date, currency, 0.00) {
  amounts_.clear();
}

MoneyArray::MoneyArray(const QDate& date, const QString& money_str)
  : Money(date, Currency::USD, 0.00) {
  for (const QString &moneyString : money_str.split(", ")) {
    Money money(date_, moneyString);
    changeCurrency(money.currency());
    push_back(money);
  }
}

MoneyArray::MoneyArray(const Money& money)
  : Money(money.date_, money.currency(), 0.00) {
  amounts_.push_back(money.amount_);
}

MoneyArray MoneyArray::operator -() const {
  MoneyArray moneyArray(*this);
  for (int i = 0; i < amounts_.size(); i++) {
    moneyArray.amounts_[i] = -moneyArray.amounts_.at(i);
  }
  return moneyArray;
}

bool MoneyArray::isZero() const
{
    for (const double &amount : amounts_)
        if (qFabs(amount) > 1e-4)
            return false;
    return true;
}

Money MoneyArray::sum() const {
  Money money(date_, currency_, 0);
  for (const double &amount : amounts_)
    money.amount_ += amount;
  return money;
}

Money MoneyArray::getMoney(int index) const
{
    Money money(date_, currency_, 0);
    if (index < amounts_.size())
        money.amount_ = amounts_.at(index);
    return money;
}

void MoneyArray::push_back(Money money) {
  if (amounts_.isEmpty()) {
    date_ = money.date_;
  }
  money.changeCurrency(currency_);
  amounts_.push_back(money.amount_);
}

QString MoneyArray::toString() const {
  QStringList result;
  for (const double& amount : amounts_) {
    result << Money(date_, currency_, amount).toString();
  }
  return result.join(", ");
}

void MoneyArray::changeCurrency(Currency::Type currency) {
  double currencyRate = g_currency.getExchangeRate(date_, currency_, currency);
  currency_ = currency;
  for (double& amount : amounts_) {
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
    moneyArray.date_ = date_ > moneyArray.date_? date_ : moneyArray.date_;

    moneyArray.changeCurrency(currency_);

    while (moneyArray.amounts_.size() < amounts_.size())
        moneyArray.amounts_.push_back(0.00);

    for (int i = 0; i < moneyArray.amounts_.size(); i++)
    {
        if (amounts_.size() <= i)
            moneyArray.amounts_[i] = f(0              , moneyArray.amounts_.at(i));
        else
            moneyArray.amounts_[i] = f(amounts_.at(i), moneyArray.amounts_.at(i));
    }

    while (!moneyArray.amounts_.isEmpty() and moneyArray.amounts_.back() == 0.0)
        moneyArray.amounts_.pop_back();

    return moneyArray;
}
