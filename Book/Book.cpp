#include "book.h"

const QMap<Account::Type, QString> kAccountTableName =
    {{Account::Asset,     "book_assets"},
     {Account::Liability, "book_liabilities"},
     {Account::Revenue,   "book_revenues"},
     {Account::Expense,   "book_expenses"},
     {Account::Equity,    "book_equities"}};

Book::Book(const QString& dbPath) {
    QFileInfo fileInfo(dbPath);
    if (fileInfo.exists()) {
        db = QSqlDatabase::addDatabase("QSQLITE", "BOOK");
        db.setDatabaseName(fileInfo.absoluteFilePath());
        if (!db.open()) {
            qDebug() << Q_FUNC_INFO << "Error: connection with database fail.";
            return;
        }
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "BOOK");
        db.setDatabaseName(dbPath);
//        Q_INIT_RESOURCE(Book);
        QFile DDL(":/book/CreateDbBook.sql");
        if (db.open() && DDL.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString statement;
            while (!DDL.atEnd()) {
                QString line = DDL.readLine();
                statement += line;
                if (statement.contains(';')) {
                    QSqlQuery(statement, db);
                    statement.clear();
                }
            }
            DDL.close();
        } else {
            qDebug() << Q_FUNC_INFO << "Database not opened or CreateDatabase.txt not opened";
            return;
        }
    }
    QSqlQuery("PRAGMA case_sensitive_like = false", db);
    start_time_ = QDateTime::currentDateTime();

    reduceLoggingRows();

    // Some schema migration work can be done here.
    if (false) {
        QList<Transaction> transactions;
        QSqlQuery query(db);
        query.prepare(R"sql(SELECT * FROM book_transactions ORDER BY date_time ASC)sql");
        query.exec();
        while (query.next()) {
            Transaction transaction;
            transaction.date_time = query.value("date_time").toDateTime();
            transaction.description = query.value("description").toString();
            auto json_document = QJsonDocument::fromJson(query.value("detail").toString().toUtf8());
            transaction.addData(json_document.object());
            transactions.push_back(transaction);
        }
        qDebug() << transactions.size();
        int i = 20000;
        for (auto transaction : transactions) {
            QSqlQuery query(db);
            query.prepare(R"sql(UPDATE book_transactions SET transaction_id = :id WHERE date_time = :dateTime)sql");
            query.bindValue(":dateTime", transaction.date_time.toString(kDateTimeFormat));
            query.bindValue(":id", i++);
            if (!query.exec()) {
                qDebug() << "Error: " << transaction.date_time.toString(kDateTimeFormat) << query.lastError();
            }
        }
    }
}

Book::~Book() {
    logUsageTime();
    closeDatabase();
}

void Book::closeDatabase() {
    if (db.isOpen()) {
        db.close();
    }
}

bool Book::insertTransaction(int user_id, const Transaction& transaction, bool ignore_error) const {
    if (!ignore_error && !transaction.validate().isEmpty()) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"sql(INSERT INTO book_transactions (user_id, date_time, description, detail) VALUES (:user_id, :dateTime, :description, :detail))sql");
    query.bindValue(":user_id",     user_id);
    query.bindValue(":dateTime",    transaction.date_time.toString(kDateTimeFormat));
    query.bindValue(":description", transaction.description);
    query.bindValue(":detail",      QJsonDocument(transaction.toJson()).toJson(QJsonDocument::Indented).toStdString().c_str());

    if (query.exec()) {
        Logging(query);  // TODO: get the binded string from query.
        return true;
    } else {
        qDebug() << Q_FUNC_INFO << "#Error Insert a transaction:" << query.lastError().text();
        return false;
    }
}

QList<Transaction> Book::queryTransactions(const TransactionFilter& filter) const {
    QStringList statements;
    for (const Account& account : filter.getAccounts()) {
        if (!account.category.isEmpty()) {
            statements << QString(R"sql((detail->'%1'->>'%2|%3' NOT NULL))sql").arg(account.typeName(), account.category, account.name);
        }
    }

    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT * FROM book_transactions WHERE (date_time BETWEEN :startDate AND :endDate) AND (description LIKE :description))sql") +
                 (statements.empty()? "" : " AND ") +
                 statements.join(filter.use_or? " OR " : " AND ") +
                 " ORDER BY date_time " + (filter.ascending_order? "ASC" : "DESC") +
                 " LIMIT :limit");

    query.bindValue(":startDate", filter.date_time.toString(kDateTimeFormat));
    query.bindValue(":endDate",   filter.end_date_time.toString(kDateTimeFormat));
    query.bindValue(":description", "%" + filter.description + "%");
    query.bindValue(":limit", filter.limit);
    qDebug() << query.lastQuery();

    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return {};
    }

    QList<Transaction> result;
    while (query.next()) {
        Transaction transaction;
        transaction.id = query.value("transaction_id").toInt();
        transaction.description = query.value("description").toString();
        transaction.date_time = query.value("date_time").toDateTime();
        // TODO: somehow several outbounded transaction will also be selected.
        // This hard code is to remove them.
        if (transaction.date_time < filter.date_time or transaction.date_time > filter.end_date_time) {
            continue;
        }
        auto json_document = QJsonDocument::fromJson(query.value("detail").toString().toUtf8());
        transaction.addData(json_document.object());
        result.push_back(transaction);
    }

    return result;
}

QList<FinancialStat> Book::getSummaryByMonth(const QDateTime &endDateTime) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT * FROM book_transactions WHERE date_time <= :d ORDER BY date_time ASC)sql");
    query.bindValue(":d", endDateTime.toString(kDateTimeFormat));
    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
    }

  QList<FinancialStat> retSummarys;
  FinancialStat monthlySummary;
  QDate month = QDate(getFirstTransactionDateTime().date().year(), getFirstTransactionDateTime().date().month(), 1);

  while (query.next()) {
    Transaction transaction;
    transaction.date_time = query.value("date_time").toDateTime();
    auto json_document = QJsonDocument::fromJson(query.value("detail").toString().toUtf8());
    transaction.addData(json_document.object());

    // Use `while` instead of `if` in case there was no transaction for successive months.
    while (transaction.date_time.date() >= month.addMonths(1)) {
      monthlySummary.description = month.toString("yyyy-MM");
      retSummarys.push_front(monthlySummary);

      month = month.addMonths(1);
      monthlySummary.clear(Account::Revenue);
      monthlySummary.clear(Account::Expense);
    }

    // TODO: next line increate the time from 5s to 9s.
    monthlySummary.changeDate(transaction.date_time.date());  // Must run this before set m_dateTime.
    monthlySummary.date_time = transaction.date_time;
    monthlySummary += transaction;
    monthlySummary.retainedEarnings += transaction.getRetainedEarnings();
    monthlySummary.transactionError += MoneyArray(transaction.getCheckSum());
  }
  monthlySummary.description = month.toString("yyyy-MM");
  retSummarys.push_front(monthlySummary);

  return retSummarys;
}

void Book::removeTransaction(int transaction_id) const {
    QSqlQuery query(db);
    query.prepare(R"sql(DELETE FROM book_transactions WHERE transaction_id = :id)sql");
    query.bindValue(":id", transaction_id);
    if (query.exec()) {
        Logging(query);
    } else {
        qDebug() << Q_FUNC_INFO << query.lastError();
    }
}

QString Book::moveAccount(const Account& old_account, const Account& new_account) const {
    if (old_account.type != new_account.type) {
        return "Does not support move account between different account type yet.";
    }

    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT transaction_id, detail->'%1' AS type_detail
                                FROM book_transactions
                                WHERE type_detail LIKE :expression)sql").arg(old_account.typeName()));
    query.bindValue(":expression", QString(R"(%"%1|%2":%)").arg(old_account.category, old_account.name));
    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return "Error execute query.";
    }
    // Update transactions
    while (query.next()) {
        QString new_string = query.value("type_detail").toString().replace(QString(R"("%1|%2":)").arg(old_account.category, old_account.name),
                                                                           QString(R"("%1|%2":)").arg(new_account.category, new_account.name));
        QSqlQuery query2(db);
        query2.prepare(QString(R"sql(UPDATE book_transactions
                                     SET detail = json_set(detail, '$.%1', json(:new))
                                     WHERE transaction_id = :id)sql").arg(old_account.typeName()));
        query2.bindValue(":id", query.value("transaction_id"));
        query2.bindValue(":new", new_string);
        if (query2.exec()) {
            Logging(query2);
        } else {
            qDebug() << Q_FUNC_INFO << query2.lastError();
            return "Error execute query.";
        }
    }

    // Update investment section in transactions.
    if (old_account.type == Account::Asset && IsInvestment(old_account)) {
        query.prepare(R"sql(SELECT transaction_id, detail->'Revenue' AS revenue_detail
                            FROM book_transactions
                            WHERE revenue_detail LIKE :expression)sql");
        query.bindValue(":expression", QString(R"(%"Investment|%1":%)").arg(old_account.name));
        if (!query.exec()) {
            qDebug() << Q_FUNC_INFO << query.lastError();
            return "Error execute query.";
        }
        while (query.next()) {
            QString new_string = query.value("revenue_detail").toString().replace(QString(R"("Investment|%1":)").arg(old_account.name),
                                                                                  QString(R"("Investment|%1":)").arg(new_account.name));
            QSqlQuery query2(db);
            query2.prepare(R"sql(UPDATE book_transactions
                                 SET detail = json_set(detail, '$.Revenue', json(:new))
                                 WHERE transaction_id = :id)sql");
            query2.bindValue(":id", query.value("transaction_id"));
            query2.bindValue(":new", new_string);
            if (query2.exec()) {
                Logging(query2);
            } else {
                qDebug() << Q_FUNC_INFO << query2.lastError();
                return "Error execute query.";
            }
        }
    }

    // Update account.
    if (accountExist(new_account)) {  // Merge account
        // TODO: old comment is ignored.
        query.prepare(QString(R"sql(DELETE FROM [%1] WHERE Category = :c AND Name = :n)sql").arg(old_account.typeName()));
        query.bindValue(":oc", old_account.category);
        query.bindValue(":on", old_account.name);
        query.exec();
        // TODO: Investment force set to true.
        if (new_account.type == Account::Asset) {
            return setInvestment(new_account, true);
        }
    } else {  // Rename account
        query.prepare(QString(R"sql(UPDATE %1 SET Category = :nc, Name = :nn WHERE Category = :oc AND Name = :on)sql").arg(old_account.typeName()));
        query.bindValue(":nc", new_account.category);
        query.bindValue(":nn", new_account.name);
        query.bindValue(":oc", old_account.category);
        query.bindValue(":on", old_account.name);
        if (query.exec()) {
            Logging(query);
        } else {
            qDebug() << Q_FUNC_INFO << query.lastError();
            return "Error execute query.";
        }
    }

    return "";
}

Currency::Type Book::queryCurrencyType(const Account &account) const {
    if (account.type == Account::Expense or account.type == Account::Revenue or account.type == Account::Equity)
        return Currency::USD;

    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT Currency FROM [%1] WHERE Category = :c AND Name = :n)sql").arg(account.typeName()));
    query.bindValue(":c", account.category);
    query.bindValue(":n", account.name);
    if (!query.exec())
        qDebug() << Q_FUNC_INFO << query.lastError();
    if (query.next())
        return Currency::kCurrencyToCode.key(query.value("Currency").toString());
    else {
        qDebug() << Q_FUNC_INFO << "No account was found" << account.typeName() << account.category << account.name;
        return Currency::USD;
    }
}

QStringList Book::queryCategories(Account::Type account_type) const {
    QStringList categories;
    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT DISTINCT Category FROM [%1] ORDER BY Category ASC)sql").arg(kAccountTableName.value(account_type)));
    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
    }
    while (query.next()) {
        categories << query.value("Category").toString();
    }
    if (account_type == Account::Revenue) {
        categories << "Investment";
    }
    return categories;
}

QStringList Book::queryAccounts(Account::Type account_type, const QString& category) const {
  QStringList accounts;

  // Special treatment for retriving Revenue::Investment.
  if (account_type == Account::Revenue and category == "Investment") {
    for (const AssetAccount& investment : getInvestmentAccounts()) {
      accounts << investment.name;
    }
    return accounts;
  }

  QSqlQuery query(db);
  query.prepare(QString(R"sql(SELECT Name FROM [%1] WHERE Category = :c ORDER BY Name ASC)sql").arg(kAccountTableName.value(account_type)));
  query.bindValue(":c", category);

  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return {};
  }
  while (query.next()) {
    accounts << query.value("Name").toString();
  }
  return accounts;
}

QList<AssetAccount> Book::getInvestmentAccounts() const {
  QList<AssetAccount> investments;
  QSqlQuery query(db);
  if (!query.exec(R"sql(SELECT * FROM book_assets WHERE IsInvestment = True ORDER BY Name ASC)sql")) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return {};
  }
  while (query.next()) {
    investments << AssetAccount(Account::Asset,
                                query.value("Category").toString(),
                                query.value("Name").toString(),
                                query.value("Comment").toString(), true);
  }
  return investments;
}

QList<std::shared_ptr<Account>> Book::queryAllAccountsFrom(QList<Account::Type> account_types) const {
    QList<std::shared_ptr<Account>> accounts;
    QSqlQuery query(db);
    // Empty input will query all account types.
    if (account_types.empty()) {
        account_types = kAccountTableName.keys();
    }
    for (Account::Type account_type : account_types) {
        query.prepare(QString(R"sql(SELECT * FROM [%1] ORDER BY Category ASC, Name ASC)sql").arg(kAccountTableName.value(account_type)));
        if (!query.exec()) {
            qDebug() << Q_FUNC_INFO << query.lastError();
        }
        while (query.next()) {
            std::shared_ptr<Account> account = FactoryCreateAccount(account_type, query.value("Category").toString(),
                                                            query.value("Name").toString(),
                                                            query.value("Comment").toString());
            if (account_type == Account::Asset) {
                static_cast<AssetAccount*>(account.get())->is_investment = query.value("IsInvestment").toBool();
            }
            accounts << account;
        }
    }
    return accounts;
}

QStringList Book::queryAccountNamesByLastUpdate(Account::Type account_type, const QString& category, const QDateTime& date_time) const {
    QMultiMap<QDateTime, QString> accountNamesByDate;
    for (QString account_name: queryAccounts(account_type, category)) {
    QSqlQuery query(db);
        query.prepare(QString(R"sql(SELECT MAX(date_time)
                                    FROM book_transactions
                                    WHERE detail->'%1' LIKE :expression AND date_time <= :date)sql").arg(Account::kAccountTypeName.value(account_type)));
        query.bindValue(":expression", QString(R"(%"%1|%2":%)").arg(category, account_name));
        query.bindValue(":date", date_time);
        query.exec();
        if (query.next()) {
            accountNamesByDate.insert(query.value("MAX(date_time)").toDateTime(), account_name);
        } else {
            accountNamesByDate.insert(QDateTime(QDate(1990, 05, 25), QTime(0, 0, 0)), account_name);
        }
    }

    QStringList accountNames;
    for (const QString &accountName: accountNamesByDate.values()) {
        accountNames.push_front(accountName);  // This reverses the list
    }
    return accountNames;
}

bool Book::updateAccountComment(const Account& account, const QString& comment) const {
    QSqlQuery query(db);
    query.prepare(QString(R"sql(UPDATE [%1] SET Comment = :c WHERE Category = :g AND Name = :n)sql").arg(account.typeName()));
    query.bindValue(":c", comment);
    query.bindValue(":g", account.category);
    query.bindValue(":n", account.name);

    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return false;
    } else {
        return true;
    }
}

QString Book::setInvestment(const AssetAccount& asset, bool is_investment) const {
    if (!is_investment) {
    QSqlQuery query(db);
        query.prepare(R"sql(SELECT * FROM book_transactions WHERE detail->'Revenue' LIKE :expression)sql");
        query.bindValue(":expression", QString(R"json(%"Investment|%1":%)json").arg(asset.name));
        if (!query.exec()) {
            return query.lastError().text();
        }
        if (query.next()) {
            return "Error: Cannot remove " + asset.name + " from investment since there still are transactions associate with it.";
        }
    }

    QSqlQuery query(db);
    query.prepare("UPDATE book_assets SET IsInvestment = :i WHERE Category == :g AND Name = :n");
    query.bindValue(":g", asset.category);
    query.bindValue(":n", asset.name);
    query.bindValue(":i", is_investment);

    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return query.lastError().text();
    } else {
        return "";
    }
}

bool Book::IsInvestment(const Account& account) const {
  if (account.type != Account::Asset) {
    return false;
  }

  QSqlQuery query(db);
  query.prepare(QString(R"sql(SELECT IsInvestment FROM [%1] WHERE Category = :c AND Name = :n)sql").arg(account.typeName()));
  query.bindValue(":c", account.category);
  query.bindValue(":n", account.name);
  query.exec();
  if (query.next()) {
    return query.value("IsInvestment").toBool();
  }
  return false;
}

bool Book::insertCategory(Account::Type account_type, const QString& category) const {
  if (!queryAccounts(account_type, category).empty()) {
    return false;
  }
  QSqlQuery query(db);
  query.prepare(QString(R"sql(INSERT INTO [%1] (Category, Name) VALUES (:c, :c))sql").arg(kAccountTableName.value(account_type)));
  query.bindValue(":c", category);

  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return false;
  } else {
    return true;
  }
}


bool Book::categoryExist(Account::Type account_type, const QString &category) const {
    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT * FROM [%1] WHERE Category = :c)sql").arg(kAccountTableName.value(account_type)));
    query.bindValue(":c", category);
    query.exec();
    return query.next();
}

bool Book::renameCategory(Account::Type account_type, const QString& category, const QString& new_category) const {
    if (categoryExist(account_type, new_category)) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT transaction_id, detail->'%1' AS type_detail
                                FROM book_transactions
                                WHERE type_detail LIKE :expression)sql").arg(kAccountTableName.value(account_type)));
    query.bindValue(":expression", QString(R"(%"%1|%)").arg(category));
    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return false;
    }
    while (query.next()) {
        QString new_string = query.value("type_detail").toString().replace(QString(R"("%1|)").arg(category),
                                                                           QString(R"("%1|)").arg(new_category));
        QSqlQuery query2(db);
        query2.prepare(QString(R"sql(UPDATE book_transactions
                                     SET detail = json_set(detail, '$.%1', json(:new))
                                     WHERE transaction_id = :id)sql").arg(kAccountTableName.value(account_type)));
        query2.bindValue(":id", query.value("transaction_id"));
        query2.bindValue(":new", new_string);
        if (!query2.exec()) {
            qDebug() << Q_FUNC_INFO << query2.lastError();
        }
    }

    query.prepare(QString(R"sql(UPDATE %1 SET Category = :new WHERE Category = :c)sql").arg(kAccountTableName.value(account_type)));
    query.bindValue(":c", category);
    query.bindValue(":new", new_category);
    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
    }
    return true;
}

bool Book::insertAccount(const Account& account) const {
  if (queryAccounts(account.type, account.category).empty()) {
    qDebug() << Q_FUNC_INFO << "Category " << account.category << " does not exist";
    return false;
  }

  QSqlQuery query(db);
  query.prepare(QString(R"sql(INSERT INTO [%1] (Category, Name) VALUES (:c, :n))sql").arg(account.typeName()));
  query.bindValue(":c", account.category);
  query.bindValue(":n", account.name);
  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << 2 << query.lastError();
    return false;
  }
  return true;
}

// TODO: if account doesn't exist, return true or false?
bool Book::removeAccount(const Account &account) const {
  QSqlQuery query(QString(R"sql(SELECT count(*) FROM book_transactions WHERE detail->'%1'->>'%2|%3' NOT NULL)sql").arg(account.typeName(), account.category, account.name), db);
    if (query.next()) {
        if (query.value(0).toInt() > 0) {
            return false;     // Has transactions still related with this account.
        }
    } else {
        return false;
    }

    query.prepare(QString(R"sql(DELETE FROM [%1] WHERE Category = :c AND Name = :n)sql").arg(account.typeName()));
    query.bindValue(":c", account.category);
    query.bindValue(":n", account.name);
    return query.exec();
}

bool Book::accountExist(const Account& account) const {
    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT * FROM [%1] WHERE Category = :c AND Name = :n)sql").arg(account.typeName()));
    query.bindValue(":c", account.category);
    query.bindValue(":n", account.name);
    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return false;
    }
    return query.next();
}

QDateTime Book::getFirstTransactionDateTime() const {
    QDateTime dateTime;
    QSqlQuery query(R"sql(SELECT MIN(date_time) FROM book_transactions)sql", db);
    if (query.next()) {
        dateTime = query.value("MIN(date_time)").toDateTime();
        if (!dateTime.isValid())
            dateTime = QDateTime::currentDateTime();
    }
    return dateTime;
}

QDateTime Book::getLastTransactionDateTime() const
{
    QDateTime dateTime;
    QSqlQuery query(R"sql(SELECT MAX(date_time) FROM book_transactions)sql", db);
    if (query.next()) {
        dateTime = query.value("MAX(date_time)").toDateTime();
        if (!dateTime.isValid())
            dateTime = QDateTime::currentDateTime();
    }
    return dateTime;
}

void Book::logUsageTime() {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT * FROM [Log Time] WHERE Date = :d)sql");
    query.bindValue(":d", start_time_.date().toString("yyyy-MM-dd"));
    query.exec();
    int64_t seconds = 0;
    if (query.next()) {
        seconds = query.value("Seconds").toInt();
    }
    seconds += start_time_.secsTo(QDateTime::currentDateTime());
    query.prepare(R"sql(INSERT OR REPLACE INTO [Log Time] (Date, Seconds) VALUES (:d, :s))sql");
    query.bindValue(":d", start_time_.date().toString("yyyy-MM-dd"));
    query.bindValue(":s", seconds);
    query.exec();
}

void Book::reduceLoggingRows() {
    QSqlQuery query(db);
    query.prepare(R"sql(DELETE FROM [Log] WHERE Time NOT IN (
                            SELECT Time FROM [Log] ORDER BY Time DESC LIMIT 100
                        ) )sql");
    query.exec();
}

bool Book::Logging(const QSqlQuery& query_log) const {
    QSqlQuery query(db);
    query.prepare(R"sql(INSERT INTO [Log] (Time, Query) VALUES (:t, :q))sql");
    query.bindValue(":t", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":q", getLastExecutedQuery(query_log));
    if (!query.exec()) {
        return false;
    }

    // TODO: make this remove excessive log featuer working right.
    query.prepare(R"sql(DELETE FROM [Log] WHERE Time NOT IN (SELECT TOP 2 Time FROM [Log]))sql");
    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return false;
    }
    return true;
}

QString Book::getLastExecutedQuery(const QSqlQuery& query) {
    QString result = query.lastQuery();
    QVariantList values = query.boundValues();
    int idx = -1;
    for(auto it = values.rbegin(); it != values.rend(); ++it) {
        QRegularExpressionMatch match;
        idx = result.lastIndexOf(QRegularExpression(R"regex(:\w+)regex"), idx, &match);
        result.replace(idx, match.captured().length(), it->toString());
//        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << result;
    }
    return result;
}

bool Book::updateLoginTime(int user_id) const {
    QSqlQuery query(db);
    query.prepare(R"sql(UPDATE auth_user
                        SET last_login = :dt
                        WHERE user_id = :id)sql");
    query.bindValue(":dt", QDateTime::currentDateTime());
    query.bindValue(":id", user_id);
    if (!query.exec()) {
        return false;
    }
    return true;
}

int Book::getLastLoggedInUserId() const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT user_id FROM auth_user ORDER BY last_login DESC LIMIT 1)sql");
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return 0;
    }
    if (query.next()) {
        return query.value("user_id").toInt();
    }
    return 0;
}
