#include "book.h"

//Book g_book;

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
      qDebug() << Q_FUNC_INFO << "Database not opened or CreateDatabase.txt not opened";
      return;
    }
  }
  QSqlQuery("PRAGMA case_sensitive_like = false", database_);
  start_time_ = QDateTime::currentDateTime();
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

bool Book::dateTimeExist(const QDateTime &dt) const
{
    QSqlQuery query(database_);
    query.prepare("SELECT * FROM Transactions WHERE Date = :d");
    query.bindValue(":d", dt.toString(kDateTimeFormat));
    query.exec();
    return query.next();
}

bool Book::insertTransaction(const Transaction &transaction) const {
  if (!transaction.validate().isEmpty()) {
    return false;
  }

  QSqlQuery query(database_);
  query.prepare("INSERT INTO Transactions (Date, Description, Expense, Revenue, Asset, Liability) "
                "VALUES (:dateTime, :description, :expense, :revenue, :asset, :liability)");
  query.bindValue(":dateTime",    transaction.date_time_.toString(kDateTimeFormat));
  query.bindValue(":description", transaction.description_);
  query.bindValue(":expense",     transaction.dataToString(Account::Expense));
  query.bindValue(":revenue",     transaction.dataToString(Account::Revenue));
  query.bindValue(":asset",       transaction.dataToString(Account::Asset));
  query.bindValue(":liability",   transaction.dataToString(Account::Liability));

  if (query.exec()) {
    logging(query);  // TODO: get the binded string from query.
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
    transaction.date_time_ = date_time;
    transaction.description_ = query.value("Description").toString();
    transaction.stringToData(Account::Expense,   query.value("Expense").toString());
    transaction.stringToData(Account::Revenue,   query.value("Revenue").toString());
    transaction.stringToData(Account::Asset,     query.value("Asset").toString());
    transaction.stringToData(Account::Liability, query.value("Liability").toString());
  }
  return transaction;
}

QList<Transaction> Book::queryTransactions(const TransactionFilter& filter) const {
  QStringList statements;
  for (const Account& account : filter.getAccounts()) {
    if (!account.category.isEmpty()) {
      statements << "(" + account.typeName() + " LIKE \"%[" + account.category + "|" + account.name + ":%\")";
    }
  }

  QSqlQuery query(database_);
  query.prepare(QString("SELECT * FROM Transactions WHERE (Date BETWEEN :startDate AND :endDate) AND (Description LIKE :description)") +
                (statements.empty()? "" : " AND ") +
                statements.join(filter.use_union_? " OR " : " AND ") +
                " ORDER BY Date " + (filter.ascending_order_? "ASC" : "DESC"));

  query.bindValue(":startDate", filter.date_time_.toString(kDateTimeFormat));
  query.bindValue(":endDate",   filter.end_date_time_.toString(kDateTimeFormat));
  query.bindValue(":description", "%" + filter.description_ + "%");

  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return {};
  }

  QList<Transaction> result;
  while (query.next()) {
    Transaction transaction;
    transaction.description_ = query.value("Description").toString();
    transaction.date_time_ = query.value("Date").toDateTime();
    // TODO: somehow several outbounded transaction will also be selected.
    // This hard code is to remove them.
    if (transaction.date_time_ < filter.date_time_ or transaction.date_time_ > filter.end_date_time_) {
      continue;
    }

    transaction.stringToData(Account::Expense,   query.value(Account::kTableName.value(Account::Expense)).toString());
    transaction.stringToData(Account::Revenue,   query.value(Account::kTableName.value(Account::Revenue)).toString());
    transaction.stringToData(Account::Asset,     query.value(Account::kTableName.value(Account::Asset)).toString());
    transaction.stringToData(Account::Liability, query.value(Account::kTableName.value(Account::Liability)).toString());
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
    transaction.date_time_ = query.value("Date").toDateTime();
    transaction.stringToData(Account::Expense,   query.value("Expense")  .toString());
    transaction.stringToData(Account::Revenue,   query.value("Revenue")  .toString());
    transaction.stringToData(Account::Asset,     query.value("Asset")    .toString());
    transaction.stringToData(Account::Liability, query.value("Liability").toString());

    // Use `while` instead of `if` in case there was no transaction for successive months.
    while (transaction.date_time_.date() >= month.addMonths(1)) {
      monthlySummary.description_ = month.toString("yyyy-MM");
      retSummarys.push_front(monthlySummary);

      month = month.addMonths(1);
      monthlySummary.clear(Account::Revenue);
      monthlySummary.clear(Account::Expense);
    }

    // TODO: next line increate the time from 5s to 9s.
    monthlySummary.changeDate(transaction.date_time_.date());  // Must run this before set m_dateTime.
    monthlySummary.date_time_ = transaction.date_time_;
    monthlySummary += transaction;
    monthlySummary.retainedEarnings += transaction.getRetainedEarnings();
    monthlySummary.transactionError += MoneyArray(transaction.getCheckSum());
  }
  monthlySummary.description_ = month.toString("yyyy-MM");
  retSummarys.push_front(monthlySummary);

  return retSummarys;
}

void Book::removeTransaction(const QDateTime &dateTime) const {
  QSqlQuery query(database_);
  query.prepare("DELETE FROM Transactions WHERE Date = :d");
  query.bindValue(":d", dateTime.toString(kDateTimeFormat));
  if (query.exec()) {
    logging(query);
  } else {
    qDebug() << Q_FUNC_INFO << query.lastError();
  }
}

bool Book::moveAccount(const Account &account, const Account &newAccount) const {
    if (account.type != newAccount.type) {
        qDebug() << Q_FUNC_INFO << "Does not support change table yet.";
        return false;
    }

    QSqlQuery query(database_);
    query.prepare("SELECT * FROM Transactions WHERE [" + account.typeName() + "] LIKE :exp");
    query.bindValue(":exp", "%[" + account.category + "|" + account.name + ": %");
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return false;
    }
  // Update transactions
  while (query.next()) {
    QString l_newString = query.value(account.typeName()).toString()
                               .replace("[" + account.category + "|" + account.name + ": ",
                                        "[" +   newAccount.category + "|" +   newAccount.name + ": ");
    QSqlQuery query2(database_);
    query2.prepare("UPDATE Transactions SET " + account.typeName() + " = :new WHERE Date = :d");
    query2.bindValue(":d", query.value("Date").toString());
    query2.bindValue(":new", l_newString);
    if (query2.exec()) {
      logging(query2);
    } else {
      qDebug() << Q_FUNC_INFO << query2.lastError();
    }
  }

    // Update account
    query.prepare("DELETE FROM " + account.typeName() + " WHERE Category = :c AND Name = :n");
    query.bindValue(":c", account.category);
    query.bindValue(":n", account.name);
    query.exec();
    if (query.exec()) {
      logging(query);
    } else {
      qDebug() << Q_FUNC_INFO << query.lastError();
    }

    query.prepare("INSERT INTO [" + account.typeName() + "] (Category, Name) VALUES (:c, :n)");
    query.bindValue(":c", newAccount.category);
    query.bindValue(":n", newAccount.name);
    if (!query.exec()) {
      qDebug() << Q_FUNC_INFO << "Insert new account failed, might be rename the account and merge."
               << query.lastError() << account.category << account.name << newAccount.category << newAccount.name;
    }
    // The reason not use UPDATE directly is considering the merge case

    return true;
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

QStringList Book::queryCategories(Account::Type p_tableType) const {
    QStringList l_categories;
    QSqlQuery l_query(database_);
    l_query.prepare("SELECT DISTINCT Category FROM [" + Account::kTableName.value(p_tableType) + "] ORDER BY Category ASC");
    if (!l_query.exec())
        qDebug() << Q_FUNC_INFO << l_query.lastError();
    while (l_query.next())
    {
        l_categories << l_query.value("Category").toString();
    }
    return l_categories;
}

QStringList Book::queryAccountNames(Account::Type account_type, const QString& category) const {
  QStringList names;
  QSqlQuery query(database_);
  query.prepare("SELECT Name FROM [" + Account::kTableName.value(account_type) + "] WHERE Category = :c ORDER BY Name ASC");
  query.bindValue(":c", category);

  if (!query.exec()) {
    qDebug() << Q_FUNC_INFO << query.lastError();
    return {};
  }
  while (query.next()) {
    names << query.value("Name").toString();
  }
  return names;
}

QList<Account> Book::queryAllAccountsFrom(QList<Account::Type> account_types) const {
  QList<Account> accounts;
  QSqlQuery query(database_);
  // Empty input will query all account types.
  if (account_types.empty()) {
    account_types = Account::kTableName.keys();
  }
  for (Account::Type account_type : account_types) {
    query.prepare("SELECT Category, Name FROM [" + Account::kTableName.value(account_type) + "] ORDER BY Name ASC");
    if (!query.exec()) {
      qDebug() << Q_FUNC_INFO << query.lastError();
    }
    while (query.next()) {
      accounts << Account(account_type, query.value("Category").toString(), query.value("Name").toString());
    }
  }
  return accounts;
}

QStringList Book::queryAccountNamesByLastUpdate(Account::Type tableType, const QString& category, const QDateTime& dateTime) const {
    QMultiMap<QDateTime, QString> accountNamesByDate;
    for (const QString &accountName: queryAccountNames(tableType, category))
    {
        QSqlQuery query(database_);
        query.prepare("SELECT MAX(Date) From Transactions WHERE " + Account::kTableName.value(tableType) + " LIKE :account AND Date <= :date");
        query.bindValue(":account", "%[" + category + "|" + accountName + ":%");
        query.bindValue(":date", dateTime.toString(kDateTimeFormat));
        query.exec();
        if (query.next())
            accountNamesByDate.insert(query.value("MAX(Date)").toDateTime(), accountName);
        else
            accountNamesByDate.insert(QDateTime(QDate(1990, 05, 25), QTime(0, 0, 0)), accountName);
    }

    QStringList accountNames;
    for (const QString &accountName: accountNamesByDate.values())
        accountNames.push_front(accountName);  // This is reverse the list
    return accountNames;
}

bool Book::insertCategory(const QString& account_type_name, const QString& category) const {
  if (!queryAccountNames(Account::kTableName.key(account_type_name), category).empty()) {
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

bool Book::removeCategory(const QString &tableName, const QString &category) const
{
    QSqlQuery query(database_);
    query.prepare("DELETE FROM [" + tableName + "] WHERE Category = :c AND Name IS NULL");
    query.bindValue(":c", category);
    query.exec();

    query.prepare("SELECT * FROM [" + tableName + "] WHERE Category = :c");
    query.bindValue(":c", category);
    query.exec();

    return !query.next();
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
  if (queryAccountNames(account.type, account.category).empty()) {
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

bool Book::logging(const QSqlQuery& query_log) const {
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
