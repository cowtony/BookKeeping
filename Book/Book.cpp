#include "book.h"

Book::Book(const QString& dbPath) {
    QFileInfo fileInfo(dbPath);
    if (fileInfo.exists()) {
        database_ = QSqlDatabase::addDatabase("QSQLITE", "BOOK");
        database_.setDatabaseName(fileInfo.absoluteFilePath());
        if (!database_.open()) {
            qDebug() << Q_FUNC_INFO << "Error: connection with database fail.";
            return;
        }
    } else {
        database_ = QSqlDatabase::addDatabase("QSQLITE", "BOOK");
        database_.setDatabaseName(dbPath);
        Q_INIT_RESOURCE(Book);
        QFile DDL(":/CreateDbBook.sql");
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
            qDebug() << Q_FUNC_INFO << "Database not opened or CreateDatabase.txt not opened";
            return;
        }
    }
    QSqlQuery("PRAGMA case_sensitive_like = false", database_);
    start_time_ = QDateTime::currentDateTime();

    // Some schema migration work can be done here.
    if (false) {
        QList<Transaction> transactions;
        QSqlQuery query(database_);
        query.prepare("SELECT * FROM Transactions");
        query.exec();
        while (query.next()) {
            Transaction transaction;
            transaction.date_time = query.value("Date").toDateTime();
            transaction.description = query.value("Description").toString();
            if (query.value("detail").isNull()) {
                transaction.stringToData(Account::Expense,   query.value(Account::kTableName.value(Account::Expense)).toString());
                transaction.stringToData(Account::Revenue,   query.value(Account::kTableName.value(Account::Revenue)).toString());
                transaction.stringToData(Account::Asset,     query.value(Account::kTableName.value(Account::Asset)).toString());
                transaction.stringToData(Account::Liability, query.value(Account::kTableName.value(Account::Liability)).toString());
            }
            else {
                auto json_document = QJsonDocument::fromJson(query.value("detail").toString().toUtf8());
                transaction.setData(json_document.object());
            }
            transactions.push_back(transaction);
        }
        qDebug() << transactions.size();
        for (auto transaction : transactions) {
            QSqlQuery query(database_);
            query.prepare("UPDATE Transactions SET detail = :detail, Expense = 'Empty', Revenue = 'Empty', Liability = 'Empty', Asset = 'Empty' WHERE Date = :dateTime");
            query.bindValue(":dateTime",    transaction.date_time.toString(kDateTimeFormat));
            query.bindValue(":detail",      QJsonDocument(transaction.toJson()).toJson(QJsonDocument::Indented).toStdString().c_str());
            if (!query.exec()) {
                qDebug() << "Error: " << transaction.date_time;
            }
        }
    }
}

Book::~Book() {
    logUsageTime();
    closeDatabase();
}

void Book::closeDatabase() {
    if (database_.isOpen()) {
        database_.close();
    }
}

bool Book::dateTimeExist(const QDateTime &dt) const {
    QSqlQuery query(database_);
    query.prepare("SELECT * FROM Transactions WHERE Date = :d");
    query.bindValue(":d", dt.toString(kDateTimeFormat));
    query.exec();
    return query.next();
}

bool Book::insertTransaction(const Transaction& transaction, bool ignore_error) const {
    if (!ignore_error && !transaction.validate().isEmpty()) {
        return false;
    }

    QSqlQuery query(database_);
    query.prepare("INSERT INTO Transactions (Date,  Description, detail) VALUES (:dateTime, :description, :detail)");
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

Transaction Book::queryTransaction(const QDateTime &date_time) const {
    Transaction transaction;

    QSqlQuery query(database_);
    query.prepare("SELECT * FROM Transactions WHERE Date = :d");
    query.bindValue(":d", date_time.toString(kDateTimeFormat));
    query.exec();
    if (query.next()) {
        transaction.date_time = date_time;
        transaction.description = query.value("Description").toString();
        if (query.value("detail").isNull()) {
            transaction.stringToData(Account::Expense,   query.value(Account::kTableName.value(Account::Expense)).toString());
            transaction.stringToData(Account::Revenue,   query.value(Account::kTableName.value(Account::Revenue)).toString());
            transaction.stringToData(Account::Asset,     query.value(Account::kTableName.value(Account::Asset)).toString());
            transaction.stringToData(Account::Liability, query.value(Account::kTableName.value(Account::Liability)).toString());
        } else {
            auto json_document = QJsonDocument::fromJson(query.value("detail").toString().toUtf8());
            transaction.setData(json_document.object());
        }
    }
    return transaction;
}

QList<Transaction> Book::queryTransactions(const TransactionFilter& filter) const {
    QStringList statements;
    for (const Account& account : filter.getAccounts()) {
        if (!account.category.isEmpty()) {
//            statements << "(" + account.typeName() + " LIKE \"%[" + account.category + "|" + account.name + ":%\")";  // TODO: depreacted, this is for the OLD 4 columns (Expense, Asset, etc.) schema.
            statements << QString(R"sql((json_extract(detail, '$.%1.%2|%3') NOT NULL))sql").arg(account.typeName(), account.category, account.name);
        }
    }

    QSqlQuery query(database_);
    query.prepare(QString("SELECT * FROM Transactions WHERE (Date BETWEEN :startDate AND :endDate) AND (Description LIKE :description)") +
                 (statements.empty()? "" : " AND ") +
                 statements.join(filter.use_or_? " OR " : " AND ") +
                 " ORDER BY Date " + (filter.ascending_order_? "ASC" : "DESC") +
                 " LIMIT :limit");

    query.bindValue(":startDate", filter.date_time.toString(kDateTimeFormat));
    query.bindValue(":endDate",   filter.end_date_time_.toString(kDateTimeFormat));
    query.bindValue(":description", "%" + filter.description + "%");
    query.bindValue(":limit", filter.limit_);
    qDebug() << query.lastQuery();



    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return {};
    }

    QList<Transaction> result;
    while (query.next()) {
        Transaction transaction;
        transaction.description = query.value("Description").toString();
        transaction.date_time = query.value("Date").toDateTime();
        // TODO: somehow several outbounded transaction will also be selected.
        // This hard code is to remove them.
        if (transaction.date_time < filter.date_time or transaction.date_time > filter.end_date_time_) {
            continue;
        }
        if (query.value("detail").isNull()) {
            transaction.stringToData(Account::Expense,   query.value(Account::kTableName.value(Account::Expense)).toString());
            transaction.stringToData(Account::Revenue,   query.value(Account::kTableName.value(Account::Revenue)).toString());
            transaction.stringToData(Account::Asset,     query.value(Account::kTableName.value(Account::Asset)).toString());
            transaction.stringToData(Account::Liability, query.value(Account::kTableName.value(Account::Liability)).toString());
        } else {
            auto json_document = QJsonDocument::fromJson(query.value("detail").toString().toUtf8());
            transaction.setData(json_document.object());
        }
        result.push_back(transaction);
    }

    return result;
}

QList<FinancialStat> Book::getSummaryByMonth(const QDateTime &endDateTime) const {
  QSqlQuery query(database_);
  query.prepare("SELECT * FROM Transactions WHERE Date <= :d ORDER BY Date ASC");
  query.bindValue(":d", endDateTime.toString(kDateTimeFormat));
  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
  }

  QList<FinancialStat> retSummarys;
  FinancialStat monthlySummary;
  QDate month = QDate(getFirstTransactionDateTime().date().year(), getFirstTransactionDateTime().date().month(), 1);

  while (query.next()) {
    Transaction transaction;
    transaction.date_time = query.value("Date").toDateTime();
    if (query.value("detail").isNull()) {
        transaction.stringToData(Account::Expense,   query.value(Account::kTableName.value(Account::Expense)).toString());
        transaction.stringToData(Account::Revenue,   query.value(Account::kTableName.value(Account::Revenue)).toString());
        transaction.stringToData(Account::Asset,     query.value(Account::kTableName.value(Account::Asset)).toString());
        transaction.stringToData(Account::Liability, query.value(Account::kTableName.value(Account::Liability)).toString());
    } else {
        auto json_document = QJsonDocument::fromJson(query.value("detail").toString().toUtf8());
        transaction.setData(json_document.object());
    }

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

void Book::removeTransaction(const QDateTime &dateTime) const {
  QSqlQuery query(database_);
  query.prepare("DELETE FROM Transactions WHERE Date = :d");
  query.bindValue(":d", dateTime.toString(kDateTimeFormat));
  if (query.exec()) {
    Logging(query);
  } else {
    qDebug() << Q_FUNC_INFO << query.lastError();
  }
}

QString Book::moveAccount(const Account& old_account, const Account& new_account) const {
  if (old_account.type != new_account.type) {
    return "Does not support change table yet.";
  }

  QSqlQuery query(database_);
  query.prepare("SELECT * FROM Transactions WHERE [" + old_account.typeName() + "] LIKE :exp");
  query.bindValue(":exp", "%[" + old_account.category + "|" + old_account.name + ": %");
  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return "Error execute query.";
  }
  // Update transactions
  while (query.next()) {
    QString new_string = query.value(old_account.typeName()).toString()
                              .replace("[" + old_account.category + "|" + old_account.name + ": ",
                                       "[" + new_account.category + "|" + new_account.name + ": ");
    QSqlQuery query2(database_);
    query2.prepare("UPDATE Transactions SET " + old_account.typeName() + " = :new WHERE Date = :d");
    query2.bindValue(":d", query.value("Date").toString());
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
    query.prepare("SELECT * FROM Transactions WHERE Revenue LIKE :exp");
    query.bindValue(":exp", "%[Investment|" + old_account.name + ": %");
    if (!query.exec()) {
      qDebug() << Q_FUNC_INFO << query.lastError();
      return "Error execute query.";
    }
    while (query.next()) {
      QString new_string = query.value("Revenue").toString()
                                .replace("[Investment|" + old_account.name + ": ",
                                         "[Investment|" + new_account.name + ": ");
      QSqlQuery query2(database_);
      query2.prepare("UPDATE Transactions SET Revenue = :new WHERE Date = :d");
      query2.bindValue(":d", query.value("Date").toString());
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
    query.prepare("DELETE FROM [" + old_account.typeName() + "] WHERE Category=:c AND Name=:n");
    query.bindValue(":oc", old_account.category);
    query.bindValue(":on", old_account.name);
    query.exec();
    // TODO: Investment force set to true.
    if (new_account.type == Account::Asset) {
      return setInvestment(new_account, true);
    }
  } else {  // Rename account
    query.prepare("UPDATE " + old_account.typeName() + " SET Category=:nc, Name=:nn WHERE Category=:oc AND Name=:on");
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

    QSqlQuery query(database_);
    query.prepare("SELECT Currency FROM [" + account.typeName() + "] WHERE Category = :c AND Name = :n");
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
  QSqlQuery query(database_);
  query.prepare("SELECT DISTINCT Category FROM [" + Account::kTableName.value(account_type) + "] ORDER BY Category ASC");
  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
  }
  while (query.next()) {
    categories << query.value("Category").toString();
  }
  // TODO: remove the `contains` once Investment is deprecated from table.
  if (account_type == Account::Revenue and !categories.contains("Investment")) {
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

  QSqlQuery query(database_);
  query.prepare("SELECT Name FROM [" + Account::kTableName.value(account_type) + "] WHERE Category = :c ORDER BY Name ASC");
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
  QSqlQuery query(database_);
  if (!query.exec("SELECT * FROM Asset WHERE IsInvestment = True ORDER BY Name ASC")) {
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
  QSqlQuery query(database_);
  // Empty input will query all account types.
  if (account_types.empty()) {
    account_types = Account::kTableName.keys();
  }
  for (Account::Type account_type : account_types) {
    query.prepare("SELECT * FROM [" + Account::kTableName.value(account_type) + "] ORDER BY Category ASC, Name ASC");
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

QStringList Book::queryAccountNamesByLastUpdate(Account::Type account_type, const QString& category, const QDateTime& dateTime) const {
  QMultiMap<QDateTime, QString> accountNamesByDate;
  for (QString account_name: queryAccounts(account_type, category)) {
    QSqlQuery query(database_);
    query.prepare("SELECT MAX(Date) From Transactions WHERE " + Account::kTableName.value(account_type) + " LIKE :account AND Date <= :date");
    query.bindValue(":account", "%[" + category + "|" + account_name + ":%");
    query.bindValue(":date", dateTime.toString(kDateTimeFormat));
    query.exec();
    if (query.next()) {
      accountNamesByDate.insert(query.value("MAX(Date)").toDateTime(), account_name);
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
  QSqlQuery query(database_);
  query.prepare("UPDATE [" + account.typeName() + "] SET Comment = :c WHERE Category == :g AND Name = :n");
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
    QSqlQuery query(database_);
    query.prepare("SELECT * FROM Transactions WHERE Revenue LIKE :exp");
    query.bindValue(":exp", "%[Investment|" + asset.name + ":%");
    if (!query.exec()) {
      return query.lastError().text();
    }
    if (query.next()) {
      return "Error: Cannot remove " + asset.name + " from investment since there still are transactions associate with it.";
    }
  }
  QSqlQuery query(database_);
  query.prepare("UPDATE Asset SET IsInvestment = :i WHERE Category == :g AND Name = :n");
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

  QSqlQuery query(database_);
  query.prepare("SELECT IsInvestment FROM [" + account.typeName() + "] WHERE Category = :c AND Name = :n");
  query.bindValue(":c", account.category);
  query.bindValue(":n", account.name);
  query.exec();
  if (query.next()) {
    return query.value("IsInvestment").toBool();
  }
  return false;
}

bool Book::insertCategory(const QString& account_type_name, const QString& category) const {
  if (!queryAccounts(Account::kTableName.key(account_type_name), category).empty()) {
    return false;
  }
  QSqlQuery query(database_);
  query.prepare("INSERT INTO [" + account_type_name + "] (Category, Name) VALUES (:c, :c)");
  query.bindValue(":c", category);

  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return false;
  } else {
    return true;
  }
}


bool Book::categoryExist(const QString &tableName, const QString &category) const
{
    QSqlQuery query(database_);
    query.prepare("SELECT * FROM [" + tableName + "] WHERE Category = :c");
    query.bindValue(":c", category);
    query.exec();
    return query.next();
}

bool Book::renameCategory(const QString &tableName, const QString &category, const QString &newCategory) const {
    if (categoryExist(tableName, newCategory))
        return false;

    QSqlQuery query(database_);
    query.prepare("SELECT * FROM Transactions WHERE [" + tableName + "] LIKE :exp");
    query.bindValue(":exp", "%[" + category + "|%");
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return false;
    }
    while (query.next())
    {
        QString newString = query.value(tableName).toString().replace("[" + category + "|", "[" + newCategory + "|");
        QSqlQuery query2(database_);
        query2.prepare("UPDATE Transactions SET " + tableName + " = :new WHERE Date = :d");
        query2.bindValue(":d", query.value("Date").toString());
        query2.bindValue(":new", newString);
        if (!query2.exec())
            qDebug() << Q_FUNC_INFO << query2.lastError();
    }

    query.prepare("UPDATE " + tableName + " SET Category = :new WHERE Category = :c");
    query.bindValue(":c", category);
    query.bindValue(":new", newCategory);
    query.exec();
    if (!query.exec())
        qDebug() << Q_FUNC_INFO << query.lastError();

    return true;
}

bool Book::insertAccount(const Account& account) const {
  if (queryAccounts(account.type, account.category).empty()) {
    qDebug() << Q_FUNC_INFO << "Category " << account.category << " does not exist";
    return false;
  }

  QSqlQuery query(database_);
  query.prepare("INSERT INTO [" + account.typeName() + "] (Category, Name) VALUES (:c, :n)");
  query.bindValue(":c", account.category);
  query.bindValue(":n", account.name);
  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << 2 << query.lastError();
    return false;
  }
  return true;
}

// TODO: if account doesn't exist, return true or false?
bool Book::removeAccount(const Account &account) const
{
    QSqlQuery l_query(database_);
    l_query.prepare("SELECT * FROM Transactions WHERE [" + account.typeName() + "] LIKE :exp");
    l_query.bindValue(":exp", "%[" + account.category + "|" + account.name + ":%");
    if (!l_query.exec())
    {
        qDebug() << Q_FUNC_INFO << l_query.lastError();
        return false;
    }
    if (l_query.next())
        return false;
    else
    {
        l_query.prepare("DELETE FROM [" + account.typeName() + "] WHERE Category = :c AND Name = :n");
        l_query.bindValue(":c", account.category);
        l_query.bindValue(":n", account.name);
        return l_query.exec();
    }
}

bool Book::accountExist(const Account& account) const {
  QSqlQuery query(database_);
  query.prepare("SELECT * FROM [" + account.typeName() + "] WHERE Category = :c AND Name = :n");
  query.bindValue(":c", account.category);
  query.bindValue(":n", account.name);
  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return false;
  }
  return query.next();
}

QDateTime Book::getFirstTransactionDateTime() const
{
    QDateTime dateTime;
    QSqlQuery query("SELECT MIN(Date) FROM Transactions", database_);
    if (query.next()) {
        dateTime = query.value("MIN(Date)").toDateTime();
        if (!dateTime.isValid())
            dateTime = QDateTime::currentDateTime();
    }
    return dateTime;
}

QDateTime Book::getLastTransactionDateTime() const
{
    QDateTime dateTime;
    QSqlQuery query("SELECT MAX(Date) FROM Transactions", database_);
    if (query.next()) {
        dateTime = query.value("MAX(Date)").toDateTime();
        if (!dateTime.isValid())
            dateTime = QDateTime::currentDateTime();
    }
    return dateTime;
}

void Book::logUsageTime() {
  QSqlQuery query(database_);
  query.prepare("SELECT * FROM [Log Time] WHERE Date = :d");
  query.bindValue(":d", start_time_.date().toString("yyyy-MM-dd"));
  query.exec();
  int64_t seconds = 0;
  if (query.next()) {
    seconds = query.value("Seconds").toInt();
  }
  seconds += start_time_.secsTo(QDateTime::currentDateTime());
  query.prepare("INSERT OR REPLACE INTO [Log Time] (Date, Seconds) VALUES (:d, :s)");
  query.bindValue(":d", start_time_.date().toString("yyyy-MM-dd"));
  query.bindValue(":s", seconds);
  query.exec();
}

bool Book::Logging(const QSqlQuery& query_log) const {
  QSqlQuery query(database_);
  query.prepare("INSERT INTO [Log] (Time, Query) VALUES (:t, :q)");
  query.bindValue(":t", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
  query.bindValue(":q", getLastExecutedQuery(query_log));
  if (!query.exec()) {
    return false;
  }

  // TODO: make this remove excessive log featuer working right.
  query.prepare("DELETE FROM [Log] WHERE Time NOT IN (SELECT TOP 2 Time FROM [Log])");
  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return false;
  }
  return true;
}

QString Book::getLastExecutedQuery(const QSqlQuery& query) {
  QString str = query.lastQuery();

  QVariantList bound_values(query.boundValues());
  qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << bound_values.back().toString();
  // TODO: Fix the following section.
  /*
  QMapIterator<QString, QVariant> it(query.boundValues());

  it.toBack();

  while (it.hasPrevious()) {
    it.previous();
    str.replace(it.key(), it.value().toString());
  }
  */
  return str;
}
