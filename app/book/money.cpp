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
    money.changeCurrency(currency_type_);  // Align to existing currency type.
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

void Money::operator +=(const Money& money) {
    *this = *this + money;
}

void Money::operator -=(const Money& money) {
    *this = *this - money;
}

Money::operator QString() const {
    // Example of negative: ($100.00)
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
    return qFabs(amount_) < 0.005;
}

/*************** HouseholdMoney ********************/
HouseholdMoney::HouseholdMoney(const QDate& date, Currency::Type type)
    : currency_type_(type), date_(date) {}

HouseholdMoney::HouseholdMoney(const QString& household, const Money& money) {
    data_[household] = money;
    date_ = money.date_;
    currency_type_ = money.currency();
}

Money HouseholdMoney::sum() const {
    Money result(date_, currency_type_);
    for (const auto& [_, money] : data_.asKeyValueRange()) {
        result += money;
    }
    return result;
}

Currency::Type HouseholdMoney::currencyType() const {
    return currency_type_;
}

const QHash<QString, Money>& HouseholdMoney::data() const {
    return data_;
}

HouseholdMoney HouseholdMoney::operator +(const HouseholdMoney& household_money) const {
    HouseholdMoney result = *this;
    for (const auto& [household, money] : household_money.data().asKeyValueRange()) {
        result.add(household, money);
    }
    return result;
}

void HouseholdMoney::operator +=(const HouseholdMoney& household_money) {
    *this = *this + household_money;
}

void HouseholdMoney::changeCurrency(Currency::Type new_currency_type) {
    currency_type_ = new_currency_type;
    for (const QString& name : data_.keys()) {
        data_[name].changeCurrency(new_currency_type);
    }
}

void HouseholdMoney::add(const QString& household, Money money) {
    Q_ASSERT(money.currency() == currency_type_);
//    money.changeCurrency(currency_type_);
    data_[household] += money;
    removeIfZero(household);
}

void HouseholdMoney::minus(const QString &household, Money money) {
    Q_ASSERT(money.currency() == currency_type_);
//    money.changeCurrency(currency_type_);
    data_[household] -= money;
    removeIfZero(household);
}

void HouseholdMoney::removeIfZero(const QString &household) {
    if (data_.contains(household) && data_.value(household).isZero()) {
        data_.remove(household);
    }
}
