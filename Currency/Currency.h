#ifndef CURRENCY_H
#define CURRENCY_H

#if defined(CURRENCY_LIBRARY)
#  define CURRENCYSHARED_EXPORT __declspec(dllexport)
#else
#  define CURRENCYSHARED_EXPORT __declspec(dllimport)
#endif

#include <QtSql>
#include <QObject>
#include <QNetworkReply>

// TODO: Software will corrupt if Currency.db does not exist.

class CURRENCYSHARED_EXPORT Currency : public QObject {
  Q_OBJECT
public:
  Currency(QObject *parent = nullptr);
  ~Currency();

  typedef enum {EUR, USD, CNY, GBP} Type;
  static const QMap<Type, QString> kCurrencyToSymbol;  // Example: "$".
  static const QMap<Type, QString> kCurrencyToCode;  // Three letter code such as "USD".

  void openDatabase(const QString& p_dbPath = "Currency.db");
  void closeDatabase();
  double getExchangeRate(const QDate& date, Type fromSymbol, Type toSymbol);

private slots:
  void onNetworkReply(QNetworkReply*);

private:
  QSqlDatabase database_;
  QNetworkAccessManager web_ctrl_;
  QSet<QDate> requested_date_;

  void removeInvalidCurrency();
};

CURRENCYSHARED_EXPORT extern Currency g_currency;

#endif // CURRENCY_H
