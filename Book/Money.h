#ifndef MONEY_H
#define MONEY_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include "currency.h"

const int PERSON_COUNT = 2;

class BOOKSHARED_EXPORT Money {
  public:
    explicit Money(const QDate& date, Currency::Type currency = Currency::USD, double amount = 0.00);
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

    Money round() const;

    Currency::Type currency() const;
    Money& changeCurrency(Currency::Type currency_type);

  protected:
    Currency::Type currency_type_; // Making this private because change this value will cause m_amounts change as well.

  private:
    double getRoundedAmount() const;
};

class BOOKSHARED_EXPORT MoneyArray : public Money {
  public:
    explicit MoneyArray(const QDate& date = QDate(1990, 05, 25), Currency::Type currency = Currency::USD);
    explicit MoneyArray(const QDate& date, const QString& p_money_s);
    explicit MoneyArray(const Money& money);

    QVector<double> amounts_;

    bool isZero() const;
    Money sum() const;
    Money getMoney(int index) const;
    void push_back(Money money);
    operator QString() const;

    void changeCurrency(Currency::Type currency_type);

    MoneyArray operator -() const;
    MoneyArray operator  +(const MoneyArray& moneyArray) const;
    void       operator +=(const MoneyArray& moneyArray);
    MoneyArray operator  -(const MoneyArray& moneyArray) const;
    void       operator -=(const MoneyArray& moneyArray);

  private:
    MoneyArray addMinus(MoneyArray moneyArray, double f(double, double)) const;
};


#endif // MONEY_H
