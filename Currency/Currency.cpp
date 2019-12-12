#include "Currency.h"
#include <QFile>
#include <QDebug>
#include <QUrl>
#include <QtSql>

const QMap<Currency_e, QString> Currency::Symbol_1 = {{USD, "$"},   {CNY, "¥"},   {EUR, "€"},   {GBP, "£"}};
const QMap<Currency_e, QString> Currency::Symbol_3 = {{USD, "USD"}, {CNY, "CNY"}, {EUR, "EUR"}, {GBP, "GBP"}};

Currency g_currency;

Currency::Currency(QObject *parent) : QObject(parent)
{
    connect(&webCtrl, &QNetworkAccessManager::finished, this, &Currency::onNetworkReply);
}

Currency::~Currency()
{
    closeDatabase();
}

void Currency::openDatabase(const QString &dbPath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE", "CURRENCY");
    m_database.setDatabaseName(dbPath);
    if (!m_database.open())
    {
        qDebug() << Q_FUNC_INFO << m_database.lastError();
    }
    removeInvalidCurrency();
}

void Currency::closeDatabase()
{
    if (m_database.isOpen())
        m_database.close();
}

double Currency::getCurrencyRate(const QDate& date, const Currency_e &fromSymbol, const Currency_e &toSymbol)
{
    if (fromSymbol == toSymbol)
        return 1.0;

    double result = 0.0 / 0.0;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM Currency WHERE Date <= :d ORDER BY Date DESC");
    query.bindValue(":d", date.toString("yyyy-MM-dd"));
    if (!query.exec())
        qDebug() << Q_FUNC_INFO << query.lastError();
    if (query.next())
    {
        result = query.value(Symbol_3.value(toSymbol)).toDouble() / query.value(Symbol_3.value(fromSymbol)).toDouble();
        if (query.value("Date").toDate() == date)
            return result;
    }
    else
        qDebug() << Q_FUNC_INFO << "Currency not found in date" << date;

    if (!requestedDate.contains(date) and date < QDate::currentDate())
    {
        requestedDate.insert(date);
        QUrl url("http://data.fixer.io/api/"
                 + date.toString("yyyy-MM-dd") + "?"
//                 "access_key=077dbea3a01e2c601af7d870ea30191c"  // mu.niu.525@gmail.com   19900525
                 "access_key=b6ec9d9dd5efa56c094fc370fd68fbc8"    // cowtony@163.com        19900525
//                 "access_key=af07896d862782074e282611f63bc64b"  // mniu@umich.edu         19900525
        //       "&base=EUR"    // This is for paid user only
                 "&symbols=" + Symbol_3.values().join(','));
        qDebug() << Q_FUNC_INFO << "Requesting:" << url;
        webCtrl.get(QNetworkRequest(url));
    }

    return result;
}

void Currency::removeInvalidCurrency()
{
    QSqlQuery query("DELETE FROM Currency WHERE EUR IS NULL"
                                           " OR USD IS NULL"
                                           " OR CNY IS NULL"
                                           " OR GBP IS NULL", m_database);
}

void Currency::onNetworkReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QString jsonString = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonString.toUtf8());
        QJsonObject jsonObject = jsonResponse.object();
        QDateTime dateTime;
        dateTime.setTime_t(jsonObject.value("timestamp").toInt());
        QJsonValue rates = jsonObject.value("rates");
        QJsonObject jsonRates = rates.toObject();

        QSqlQuery query(m_database);
        query.prepare("INSERT OR REPLACE INTO Currency (Date, " + Symbol_3.values().join(", ") + ") "
                                               "VALUES (:d, :" + Symbol_3.values().join(", :").toLower() + ")");
        query.bindValue(":d", dateTime.date().toString("yyyy-MM-dd"));
        for (QString symbol: Symbol_3.values())
            query.bindValue(":" + symbol.toLower(), jsonRates.value(symbol).toDouble());
        if (!query.exec())
            qDebug() << Q_FUNC_INFO << query.lastError();
    }
    else
    {
        qDebug() << Q_FUNC_INFO << "ErrorReply";
    }
    delete reply;
}
