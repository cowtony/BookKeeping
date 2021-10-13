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
// TODO: Software will crush on new date without rate into.

enum Currency_e {EUR, USD, CNY, GBP};

class CURRENCYSHARED_EXPORT Currency : public QObject {
  Q_OBJECT
public:
  Currency(QObject *parent = nullptr);
  ~Currency();

    void openDatabase(const QString& p_dbPath = "Currency.db");
    void closeDatabase();
    double getCurrencyRate(const QDate& date, Currency_e fromSymbol, Currency_e toSymbol);

    static const QMap<Currency_e, QString> Symbol_1;
    static const QMap<Currency_e, QString> Symbol_3;

private slots:
    void onNetworkReply(QNetworkReply*);

private:
  QSqlDatabase database_;
  QNetworkAccessManager webCtrl_;
  QSet<QDate> requestedDate_;

  void removeInvalidCurrency();
};

CURRENCYSHARED_EXPORT extern Currency g_currency;

#endif // CURRENCY_H
