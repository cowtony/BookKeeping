#include "currency/currency.h"

#include <QFile>
#include <QDebug>
#include <QUrl>
#include <QtSql>

#include "utils/scoped_logger.h"

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
    // Firstly, try to connect to PostgreSQL:
    db_ = QSqlDatabase::addDatabase("QPSQL", "Postgre_Currency");  // Note: To use PostgreSQL, need to add "C:\Program Files\PostgreSQL\15\lib" to system path.
    db_.setHostName("localhost");
    db_.setDatabaseName("book_keeping");
    db_.setUserName("postgres");
    db_.setPassword("19900525");
    if (db_.open()) {
        LOG_INFO() << "Connect to PostgreSQL: currency_currency";
        removeInvalidCurrency();
        fillEmptyDate(QDate::currentDate().addYears(-1));
        return true;
    } else {
        LOG_ERROR() << db_.lastError();
        db_.removeDatabase("Postgre_Currency");
    }

    // Secondly, try to connect to SQLite:
    db_ = QSqlDatabase::addDatabase("QSQLITE", "CURRENCY");
    QFileInfo fileInfo("Currency.db");
    if (fileInfo.exists()) {
        db_.setDatabaseName(fileInfo.absoluteFilePath());
        if (!db_.open()) {
            LOG_ERROR() << db_.lastError();
            return false;
        }
    } else {  // Create new DB from script.
        db_.setDatabaseName("Currency.db");
        QFile DDL(":/currency/CreateDbCurrency.sql");
        if (db_.open() && DDL.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString statement;
            while (!DDL.atEnd()) {
                QString line = DDL.readLine();
                statement += line;
                if (statement.contains(';')) {
                    QSqlQuery(statement, db_);
                    statement.clear();
                }
            }
            DDL.close();
        } else {
            LOG_ERROR() << "Database not opened or CreateDatabase.txt not opened.";
            return false;
        }
    }

    LOG_INFO() << "Connect to SQLite: currency";
    removeInvalidCurrency();
    fillEmptyDate(QDate::currentDate().addMonths(-1));
    return true;
}

void Currency::closeDatabase() {
    if (db_.isOpen()) {
        db_.close();
    }
}

double Currency::getExchangeRate(const QDate& utc_date, Type from_symbol, Type to_symbol) {
    if (from_symbol == to_symbol) {
        return 1.0;
    }

    QSqlQuery query(db_);
    // Get the most recent entry.
    query.prepare(R"sql(SELECT * FROM currency_currency WHERE "Date" <= :date ORDER BY "Date" DESC LIMIT 1)sql");
    query.bindValue(":date", utc_date);
    if (!query.exec()) {
        LOG_ERROR() << query.lastError();
        return 0.0 / 0.0;  // NaN.
    }
    if (query.next()) {
        if (query.value("Date").toDate() != utc_date) {
            LOG_ERROR() << "Currency not found in date" << utc_date << "The most recent one is " << query.value("Date").toDate();
        }
        return query.value(kCurrencyToCode.value(to_symbol)).toDouble() / query.value(kCurrencyToCode.value(from_symbol)).toDouble();
    } else {
        return 0.0 / 0.0;  // NaN.
    }
}

void Currency::removeInvalidCurrency() {
    // TODO: This query currently have error "QPSQL: Unable to create query".
    QSqlQuery query(db_);
    query.prepare(R"sql(DELETE FROM currency_currency WHERE "EUR" IS NULL OR "USD" IS NULL OR "CNY" IS NULL OR "GBP" IS NULL)sql");
    if (!query.exec()) {
        LOG_ERROR() << query.lastError();
    }
}

void Currency::fillEmptyDate(const QDate& start_date) {
    QSqlQuery query(db_);
    query.prepare(R"sql(SELECT "Date" FROM currency_currency WHERE "Date" BETWEEN :start_date AND :end_date ORDER BY "Date" ASC)sql");
    query.bindValue(":start_date", start_date);
    query.bindValue(":end_date", QDate::currentDate());
    if (!query.exec()) {
        LOG_ERROR() << query.lastError();
        return;
    }
    QSet<QDate> existing_date;
    while (query.next()) {
        existing_date.insert(query.value("Date").toDate());
    }
    for (QDate date = start_date; date < QDateTime::currentDateTimeUtc().date(); date = date.addDays(1)) {
        if (!existing_date.contains(date)) {

            QUrl url(QString("http://data.fixer.io/api/%1?access_key=%2&symbols=%3&base=EUR")
                         .arg(date.toString("yyyy-MM-dd"),
                              // access_key:
                              // "077dbea3a01e2c601af7d870ea30191c",  // mu.niu.525@gmail.com   19900525
                              // "b6ec9d9dd5efa56c094fc370fd68fbc8",  // cowtony@163.com        19900525
                              "af07896d862782074e282611f63bc64b",  // mniu@umich.edu         19900525
                              kCurrencyToCode.values().join(',')));

            LOG_INFO() << "Requesting for date:" << date << url;
            web_ctrl_.get(QNetworkRequest(url));
        }
    }
}

void Currency::onNetworkReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        LOG_ERROR() << reply->errorString();
    }
    QString jsonString = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonString.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();
    if (!jsonObject.value("success").toBool() || !jsonObject.value("historical").toBool()) {
        LOG_ERROR() << "Error fetching currency:" << jsonObject;
        return;
    } else {
        LOG_INFO() << jsonObject;
    }

    QSqlQuery query(db_);
    query.prepare(R"sql(
        INSERT INTO currency_currency ("timestamp", "Date", "EUR", "USD", "CNY", "GBP")
        VALUES (to_timestamp(:utc), :date, :eur, :usd, :cny, :gbp)
        ON CONFLICT ("Date") DO UPDATE SET
            "timestamp" = EXCLUDED."timestamp",
            "EUR" = EXCLUDED."EUR",
            "USD" = EXCLUDED."USD",
            "CNY" = EXCLUDED."CNY",
            "GBP" = EXCLUDED."GBP";
    )sql");
    query.bindValue(":utc", jsonObject.value("timestamp").toInteger());
    query.bindValue(":date", QDate::fromString(jsonObject.value("date").toString(), "yyyy-MM-dd"));
    QJsonObject jsonRates = jsonObject.value("rates").toObject();
    for (const QString& symbol: kCurrencyToCode) {
        query.bindValue(":" + symbol.toLower(), jsonRates.value(symbol).toDouble());
    }
    if (!query.exec()) {
        LOG_ERROR() << query.lastError();
    }
    reply->deleteLater();
}
