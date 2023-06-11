#include "money.h"
#include <QtMath>
#include <QLocale>
#include "currency/currency.h"

/****************** Money ****************************/
Money::Money(const QDate& date, Currency::Type currency, double amount)
    : date_(date), amount_(amount), currency_type_(currency) {}

Money::Money(const QDate& date, QString money_str, Currency::Type currency_type)
    : date_(date), amount_(0.00), currency_type_(currency_type) {
    if (money_str.isEmpty()) {
        return;
    }
    if (money_str.front() == '(' and money_str.back() == ')') {
        money_str = "-" + money_str.mid(1, money_str.length() - 2);
    }

    int sign = 1;
    if (money_str.front() == '-') {
        sign = -1;
        money_str.remove(0, 1);
    }

    if (Currency::kCurrencyToSymbol.values().contains(money_str.left(1))) {
        currency_type_ = Currency::kCurrencyToSymbol.key(money_str.left(1));
        money_str.remove(0, 1);
    } else if (Currency::kCurrencyToCode.values().contains(money_str.left(3))) {
        currency_type_ = Currency::kCurrencyToCode.key(money_str.left(3));
        money_str.remove(0, 3);
    }

    bool ok;
    amount_ = sign * QLocale(QLocale::English).toDouble(money_str, &ok);

    if (!ok) {
        qDebug() << Q_FUNC_INFO << money_str;
    }
}

Currency::Type Money::currency() const {
    return currency_type_;
}

double Money::getRoundedAmount() const {
    return qRound(amount_ * 100) / 100.0;
}

Money Money::operator -() const {
    return Money(date_, currency_type_, -amount_);
}

Money Money::operator /(int val) const {
    if (val == 0)
        qDebug() << Q_FUNC_INFO << val;
    return Money(date_, currency_type_, amount_ / val);
}

Money Money::operator +(Money money) const {
    money.date_ = qMax(date_, money.date_);
    money.changeCurrency(currency_type_);
    money.amount_ += amount_;
    return money;
}

Money Money::operator -(Money money) const {
    money.date_ = date_ > money.date_? date_ : money.date_;
    money.changeCurrency(currency_type_);
    money.amount_ = amount_ - money.amount_;
    return money;
}

Money Money::operator *(double rateOfReturn) const {
    Money money = *this;
    money.amount_ *= rateOfReturn;
    return money;
}

bool Money::operator <(Money money) const {
    money.changeCurrency(currency_type_);
    return amount_ < money.amount_;
}

void Money::operator +=(const Money &money) {
    *this = *this + money;
}

void Money::operator -=(const Money &money) {
    *this = *this - money;
}

Money::operator QString() const {
    // TODO: This returns ($100.00) instead of -$100.00 now.
    return QLocale(QLocale::English).toCurrencyString(getRoundedAmount(), Currency::kCurrencyToSymbol.value(currency_type_), 2);
}

Money& Money::changeCurrency(Currency::Type new_currency_type) {
    amount_ *= g_currency.getExchangeRate(date_, currency_type_, new_currency_type);
    currency_type_ = new_currency_type;
    return *this;
}

Money Money::round() const {
    Money money = *this;
    money.amount_ = getRoundedAmount();
    return money;
}

bool Money::isZero() const {
    return qFabs(amount_) < 1e-4;
}

/*************** Money Array ********************/
/*
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

bool MoneyArray::isZero() const {
    for (const double &amount : amounts_)
        if (qFabs(amount) > 1e-4)
            return false;
    return true;
}

void MoneyArray::push_back(Money money) {
  if (amounts_.isEmpty()) {
    date_ = money.date_;
  }
  money.changeCurrency(currency_type_);
  amounts_.push_back(money.amount_);
}

MoneyArray::operator QString() const {
    QStringList result;
    for (const double& amount : amounts_) {
        result << Money(date_, currency_type_, amount);
    }
    return result.join(", ");
}



*/

//////////////// Class HouseholdMoney ////////////////////////
HouseholdMoney::HouseholdMoney(const QDate& date, Currency::Type type)
    : currency_type_(type), date_(date) {}

HouseholdMoney::HouseholdMoney(const QString &household, const Money &money) {
    (*this)[household] = money;
    date_ = money.date_;
    currency_type_ = money.currency();
}

Money HouseholdMoney::sum() const {
    Money result;
    for (const auto& [_, money] : asKeyValueRange())
        result += money;
    return result;
}

Currency::Type HouseholdMoney::currencyType() const {
    return currency_type_;
}

HouseholdMoney HouseholdMoney::operator +(const HouseholdMoney &household_money) const {
    return addMinus(household_money, [](double x, double y){return x + y;});
}

void HouseholdMoney::operator +=(const HouseholdMoney &household_money) {
    *this = *this + household_money;
}

HouseholdMoney HouseholdMoney::operator -(const HouseholdMoney &household_money) const {
    return addMinus(household_money, [](double x, double y){return x - y;});
}

void HouseholdMoney::operator -=(const HouseholdMoney &household_money) {
    *this = *this - household_money;
}

HouseholdMoney HouseholdMoney::addMinus(HouseholdMoney household_money, double f(double, double)) const {
    HouseholdMoney result = *this;
    result.date_ = qMax(date_, household_money.date_);
    household_money.changeCurrency(currency_type_);

    for (const auto& [household, money] : household_money.asKeyValueRange()) {
        if (!result.contains(household)) {
            result[household] = Money(result.date_, currency_type_);
        }
        result[household].amount_ = f(result[household].amount_, money.amount_);
        if (result[household].isZero()) {
            result.remove(household);
        }
    }

    return result;
}

void HouseholdMoney::changeCurrency(Currency::Type new_currency_type) {
    currency_type_ = new_currency_type;
    for (const QString& name : keys()) {
        (*this)[name].changeCurrency(new_currency_type);
    }
}
