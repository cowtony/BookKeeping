#ifndef MONEY_H
#define MONEY_H

#include "currency/currency.h"

const int PERSON_COUNT = 2;

class Money {
public:
    explicit Money(const QDate& utcDate = QDate(1990, 05, 25), Currency::Type currency = Currency::USD, double amount = 0.00);
    explicit Money(const QDate& utcDate, QString money_str, Currency::Type currency = Currency::USD);  // Valid Input: 123.5 -123.5 (123.5) USD123.50 $123.50 -USD123.50 -$123.50 ($123.50)

    QDate  utcDate;
    double amount_;

    Money  operator -() const;
    Money  operator /(int val) const;
    Money  operator +(Money money) const;
    Money  operator -(Money money) const;
    Money  operator *(double rateOfReturn) const;
    bool   operator <(Money money) const;
    void   operator+=(const Money& money);
    void   operator-=(const Money& money);
    operator QString() const;

    // Round to cent 0.00
    Money round() const;
    bool isZero() const;

    Currency::Type currency() const;
    Money& changeCurrency(Currency::Type currency_type);

private:
    double getRoundedAmount() const;

    Currency::Type currency_type_; // Making this private because change this value will cause m_amounts change as well.
};

class HouseholdMoney {
public:
    explicit HouseholdMoney(const QDate& utcDate = QDate(1990, 05, 25), Currency::Type type = Currency::USD);
    explicit HouseholdMoney(const QString& household, const Money& money);

    // Getters:
    Money sum() const;
    Currency::Type currencyType() const;
    const QHash<QString, Money>& data() const;

    // Setters:
    void changeCurrency(Currency::Type new_currency_type);
    void add(const QString& household, Money money);
    void minus(const QString& household, Money money);
    void removeIfZero(const QString& household);

    HouseholdMoney operator  +(const HouseholdMoney& household_money) const;
    void           operator +=(const HouseholdMoney& household_money);  

private:
    Currency::Type currency_type_;
    QDate utcDate_;
    QHash<QString, Money> data_;
};

#endif // MONEY_H
