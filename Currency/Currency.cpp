#include "currency.h"
#include <QFile>
#include <QDebug>
#include <QUrl>
#include <QtSql>

const QMap<Currency::Type, QString> Currency::kCurrencyToSymbol = {{Currency::Type::USD, "$"},   {Currency::CNY, "¥"},   {Currency::EUR, "€"},   {Currency::GBP, "£"}};
const QMap<Currency::Type, QString> Currency::kCurrencyToCode = {{Currency::Type::USD, "USD"}, {Currency::CNY, "CNY"}, {Currency::EUR, "EUR"}, {Currency::GBP, "GBP"}};

Currency g_currency;

Currency::Currency(QObject *parent) : QObject(parent)
{
    connect(&web_ctrl_, &QNetworkAccessManager::finished, this, &Currency::onNetworkReply);
}

Currency::~Currency() {
  closeDatabase();
}

void Currency::openDatabase(const QString &dbPath) {
  QFileInfo fileInfo(dbPath);
  if (fileInfo.exists()) {
    database_ = QSqlDatabase::addDatabase("QSQLITE", "CURRENCY");
    database_.setDatabaseName(fileInfo.absoluteFilePath());
    if (!database_.open()) {
      qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << database_.lastError();
      return;
    }
  } else {
    database_ = QSqlDatabase::addDatabase("QSQLITE", "CURRENCY");
    database_.setDatabaseName(dbPath);
    Q_INIT_RESOURCE(Currency);
    QFile DDL(":/CreateDatabase.sql");
    if (database_.open() && DDL.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QString statement;
      while (!DDL.atEnd()) {
        QString line = DDL.readLine();
        statement += line;
        if (statement.contains(';')) {
          QSqlQuery(statement, database_);
          statement.clear();
        }
      }
      DDL.close();
    } else {
      qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << "Database not opened or CreateDatabase.txt not opened.";
      return;
    }
  }

  removeInvalidCurrency();
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
  query.prepare("SELECT * FROM Currency WHERE Date <= :d ORDER BY Date DESC");
  query.bindValue(":d", date.toString("yyyy-MM-dd"));
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
             "access_key=b6ec9d9dd5efa56c094fc370fd68fbc8"    // cowtony@163.com        19900525
//             "access_key=af07896d862782074e282611f63bc64b"  // mniu@umich.edu         19900525
//             "&base=EUR"    // This is for paid user only.
             "&symbols=" + kCurrencyToCode.values().join(','));
    qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << "Requesting:" << url;
    web_ctrl_.get(QNetworkRequest(url));
  }
  return result;
}

void Currency::removeInvalidCurrency() {
  qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << "\e[0m";
  QSqlQuery("DELETE FROM Currency WHERE EUR IS NULL"
                                   " OR USD IS NULL"
                                   " OR CNY IS NULL"
                                   " OR GBP IS NULL", database_);
}

void Currency::onNetworkReply(QNetworkReply* reply) {
  qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << "\e[0m";
  if (reply->error() != QNetworkReply::NoError) {
    qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << reply->errorString();
  }
  QString jsonString = reply->readAll();
  QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonString.toUtf8());
  QJsonObject jsonObject = jsonResponse.object();
  QDateTime dateTime;
  dateTime.setSecsSinceEpoch(jsonObject.value("timestamp").toInt());
  QJsonValue rates = jsonObject.value("rates");
  QJsonObject jsonRates = rates.toObject();

  QSqlQuery query(database_);
  query.prepare("INSERT OR REPLACE INTO Currency (Date, " + kCurrencyToCode.values().join(", ") + ") "
                                         "VALUES (:d, :" + kCurrencyToCode.values().join(", :").toLower() + ")");
  query.bindValue(":d", dateTime.date().toString("yyyy-MM-dd"));
  for (const QString& symbol: kCurrencyToCode) {
    query.bindValue(":" + symbol.toLower(), jsonRates.value(symbol).toDouble());
  }
  if (!query.exec()) {
    qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
  }
  reply->deleteLater();
}
