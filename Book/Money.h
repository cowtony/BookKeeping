#ifndef MONEY_H
#define MONEY_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include "Currency.h"

const int PERSON_COUNT = 2;

class BOOKSHARED_EXPORT Money
{
public:
    explicit Money(const QDate &date, const Currency_e& currency = USD, const double& amount = 0.00);
    explicit Money(const QDate &date, QString p_money_s, const Currency_e &currency = USD);  // Valid Input: 123.5 -123.5 (123.5) USD123.50 $123.50 -USD123.50 -$123.50 ($123.50)

    QDate  m_date;
    double m_amount;

    Money operator -() const;
    Money operator /(const int &val) const;
    Money operator +(Money money) const;
    Money operator -(Money money) const;
    void operator +=(const Money &money);
    void operator -=(const Money &money);

    Currency_e getCurrency() const;
    QString toString() const;
    Money round() const;
    void changeCurrency(const Currency_e& currency_e);

private:
    Currency_e m_currency;
    double getRoundedAmount() const;
};

class BOOKSHARED_EXPORT MoneyArray
{
public:
  explicit MoneyArray(const QDate &date = QDate(1990, 05, 25), const Currency_e &currency = USD);
  explicit MoneyArray(const QDate &date, const QString &p_money_s);

    QDate           m_date;
    QVector<double> m_amounts;
    Currency_e      m_currency;

    bool isZero() const;
    Money sum() const;
    Money getMoney(const int &index) const;
    void push_back(Money money);
    QString toString() const;
    void changeCurrency(const Currency_e& currency_e);

    MoneyArray operator -() const;
    MoneyArray operator  +(const MoneyArray &moneyArray) const;
    void       operator +=(const MoneyArray &moneyArray);
    MoneyArray operator  -(const MoneyArray &moneyArray) const;
    void       operator -=(const MoneyArray &moneyArray);

private:
    MoneyArray addMinus(MoneyArray moneyArray, double f(double, double)) const;
};


#endif // MONEY_H
