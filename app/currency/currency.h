#ifndef CURRENCY_H
#define CURRENCY_H

#include <QtSql>
#include <QObject>
#include <QNetworkReply>

// TODO: Software will corrupt if Currency.db does not exist.

class Currency : public QObject {
    Q_OBJECT
  public:
    Currency(QObject *parent = nullptr);
    ~Currency();

    typedef enum {EUR, USD, CNY, GBP} Type;
    static const QMap<Type, QString> kCurrencyToSymbol;  // Example: "$".
    static const QMap<Type, QString> kCurrencyToCode;  // Three letter code such as "USD".

    bool openDatabase();
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

extern Currency g_currency;

#endif // CURRENCY_H