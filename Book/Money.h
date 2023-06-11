#ifndef MONEY_H
#define MONEY_H

#include "currency/currency.h"

const int PERSON_COUNT = 2;

class Money {
  public:
    explicit Money(const QDate& date = QDate(1990, 05, 25), Currency::Type currency = Currency::USD, double amount = 0.00);
    explicit Money(const QDate& date, QString money_str, Currency::Type currency = Currency::USD);  // Valid Input: 123.5 -123.5 (123.5) USD123.50 $123.50 -USD123.50 -$123.50 ($123.50)

    QDate  date_;
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

  protected:
    Currency::Type currency_type_; // Making this private because change this value will cause m_amounts change as well.

  private:
    double getRoundedAmount() const;
};

class HouseholdMoney : public QHash<QString, Money> {
public:
    explicit HouseholdMoney(const QDate& date = QDate(1990, 05, 25), Currency::Type type = Currency::USD);
    explicit HouseholdMoney(const QString& household, const Money& money);

//    QVector<double> amounts_;

//    bool isZero() const;
    Money sum() const;
    Currency::Type currencyType() const;
//    void push_back(Money money);
//    operator QString() const;

    void changeCurrency(Currency::Type new_currency_type);

    HouseholdMoney operator -() const;
    HouseholdMoney operator  +(const HouseholdMoney& household_money) const;
    void           operator +=(const HouseholdMoney& household_money);
    HouseholdMoney operator  -(const HouseholdMoney& household_money) const;
    void           operator -=(const HouseholdMoney& household_money);

private:
    HouseholdMoney addMinus(HouseholdMoney household_money, double f(double, double)) const;

    Currency::Type currency_type_;
    QDate  date_;
};

#endif // MONEY_H
