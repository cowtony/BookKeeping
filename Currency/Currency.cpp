#include "currency.h"
#include <QFile>
#include <QDebug>
#include <QUrl>
#include <QtSql>

const QMap<Currency::Type, QString> Currency::kCurrencyToSymbol = {{Currency::Type::USD, "$"},   {Currency::CNY, "¥"},   {Currency::EUR, "€"},   {Currency::GBP, "£"}};
const QMap<Currency::Type, QString> Currency::kCurrencyToCode = {{Currency::Type::USD, "USD"}, {Currency::CNY, "CNY"}, {Currency::EUR, "EUR"}, {Currency::GBP, "GBP"}};

Currency g_currency;

Currency::Currency(QObject *parent) : QObject(parent) {
    connect(&web_ctrl_, &QNetworkAccessManager::finished, this, &Currency::onNetworkReply);
}

Currency::~Currency() {
    closeDatabase();
}

bool Currency::openDatabase() {
    // Note: To use PostgreSQL, need to add "C:\Program Files\PostgreSQL\15\lib" to system path.
    database_ = QSqlDatabase::addDatabase("QPSQL");  // PostgreSQL
    database_.setHostName("localhost");
    database_.setDatabaseName("book_keeping");
    database_.setUserName("postgres");
    database_.setPassword("19900525");

    if (!database_.open()) {
        qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << database_.lastError();
        return false;
    }

    removeInvalidCurrency();
    return true;
}

void Currency::closeDatabase() {
  if (database_.isOpen()) {
    database_.close();
  }
}

double Currency::getExchangeRate(const QDate& date, Type from_symbol, Type to_symbol) {
  if (from_symbol == to_symbol) {
    return 1.0;
  }

  QSqlQuery query(database_);
  query.prepare(R"sql(SELECT * FROM currency_currency WHERE "Date" <= :d ORDER BY "Date" DESC LIMIT 1)sql");
  query.bindValue(":d", date);
  if (!query.exec()) {
    qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
  }
  double result = 0.0 / 0.0;  // NaN.
  if (query.next()) {
    // Get the most recent exchange rate.
    result = query.value(kCurrencyToCode.value(to_symbol)).toDouble() / query.value(kCurrencyToCode.value(from_symbol)).toDouble();
    if (query.value("Date").toDate() == date) {
      return result;
    }
  }
  qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << "Currency not found in date" << date;
  if (!requested_date_.contains(date) and date < QDate::currentDate()) {
    requested_date_.insert(date);
    QUrl url("http://data.fixer.io/api/"
             + date.toString("yyyy-MM-dd") + "?"
//             "access_key=077dbea3a01e2c601af7d870ea30191c"  // mu.niu.525@gmail.com   19900525
//             "access_key=b6ec9d9dd5efa56c094fc370fd68fbc8"    // cowtony@163.com        19900525
             "access_key=af07896d862782074e282611f63bc64b"  // mniu@umich.edu         19900525
//             "&base=EUR"    // This is for paid user only.
             "&symbols=" + kCurrencyToCode.values().join(','));
    qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << "Requesting:" << url;
    web_ctrl_.get(QNetworkRequest(url));
  }
  return result;
}

void Currency::removeInvalidCurrency() {
    QSqlQuery(R"sql(DELETE FROM currency_currency WHERE EUR IS NULL"
                                                   " OR USD IS NULL"
                                                   " OR CNY IS NULL"
                                                   " OR GBP IS NULL)sql", database_);
}

void Currency::onNetworkReply(QNetworkReply* reply) {
    qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << "\e[0m";
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << reply->errorString();
    }
    QString jsonString = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    if (!jsonObject.value("success").toBool()) {
        qDebug() << jsonObject;
        return;
    }
    QDateTime dateTime;
    dateTime.setSecsSinceEpoch(jsonObject.value("timestamp").toInt());
    QJsonObject jsonRates = jsonObject.value("rates").toObject();

    QSqlQuery query(database_);
    query.prepare(R"sql(INSERT OR REPLACE INTO currency_currency ("Date", )sql" + kCurrencyToCode.values().join(", ") + ") "
                  "VALUES (:d, :" + kCurrencyToCode.values().join(", :").toLower() + ")");
    query.bindValue(":d", dateTime.date().toString("yyyy-MM-dd"));
    qDebug() << query.lastQuery();
    for (const QString& symbol: kCurrencyToCode) {
        query.bindValue(":" + symbol.toLower(), jsonRates.value(symbol).toDouble());
    }
    if (!query.exec()) {
        qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
    }
    reply->deleteLater();
}
