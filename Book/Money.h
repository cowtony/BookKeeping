#ifndef MONEY_H
#define MONEY_H

#if defined(BOOK_LIBRARY)
#  define BOOKSHARED_EXPORT __declspec(dllexport)
#else
#  define BOOKSHARED_EXPORT __declspec(dllimport)
#endif

#include "Currency.h"

const int PERSON_COUNT = 2;

class BOOKSHARED_EXPORT Money {
public:
  explicit Money(const QDate& date, const Currency_e& currency = USD, const double& amount = 0.00);
  explicit Money(const QDate& date, QString money_str, const Currency_e& currency = USD);  // Valid Input: 123.5 -123.5 (123.5) USD123.50 $123.50 -USD123.50 -$123.50 ($123.50)

  QDate  m_date;
  double m_amount;

  Money operator -() const;
  Money operator /(const int &val) const;
  Money operator +(Money money) const;
  Money operator -(Money money) const;
  void operator +=(const Money &money);
  void operator -=(const Money &money);

  QString toString() const;
  Money round() const;

  Currency_e currency() const;
  void changeCurrency(const Currency_e& currency_e);

protected:
  Currency_e m_currency; // Make this private because change this value will cause m_amounts change as well.

private:
  double getRoundedAmount() const;
};

// TODO: make this inherited from Money
class BOOKSHARED_EXPORT MoneyArray : public Money {
public:
  explicit MoneyArray(const QDate &date = QDate(1990, 05, 25), const Currency_e &currency = USD);
  explicit MoneyArray(const QDate &date, const QString &p_money_s);
  explicit MoneyArray(const Money& money);

  QVector<double> m_amounts;

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
