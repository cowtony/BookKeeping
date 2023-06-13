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

    double getExchangeRate(const QDate& date, Type from_symbol, Type to_symbol);

  private slots:
    void onNetworkReply(QNetworkReply*);

  private:
    void closeDatabase();
    void removeInvalidCurrency();
    void fillEmptyDate(const QDate& start_date);

    QSqlDatabase db_;
    QNetworkAccessManager web_ctrl_;
};

extern Currency g_currency;

#endif // CURRENCY_H
