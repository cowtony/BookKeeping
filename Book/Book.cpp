#include "Book.h"

Book g_book;

Book::Book()
{

}

Book::~Book()
{
    logEndTime();
    closeDatabase();
}

bool Book::openDatabase(const QString& dbPath)
{
    QFileInfo fileInfo(dbPath);
    if (fileInfo.exists())
    {
        m_database = QSqlDatabase::addDatabase("QSQLITE", "BOOK");
        m_database.setDatabaseName(fileInfo.absoluteFilePath());
        if (!m_database.open())
        {
            qDebug() << Q_FUNC_INFO << "Error: connection with database fail";
            return false;
        }
    }
    else
    {
        m_database = QSqlDatabase::addDatabase("QSQLITE", "BOOK");
        m_database.setDatabaseName(dbPath);
        Q_INIT_RESOURCE(Book);
        QFile DDL(":/CreateDatabase.sql");
        if (m_database.open() && DDL.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString statement;
            while (!DDL.atEnd())
            {
                QString line = DDL.readLine();
                statement += line;
                if (statement.contains(';'))
                {
                    QSqlQuery query(statement, m_database);
                    statement.clear();
                }
            }
            DDL.close();
        }
        else
        {
            qDebug() << Q_FUNC_INFO << "Database not opened or CreateDatabase.txt not opened";
            return false;
        }
    }
    QSqlQuery query("PRAGMA case_sensitive_like = false", m_database);
    logStartTime();
    return true;
}

void Book::closeDatabase()
{
    if (m_database.isOpen())
        m_database.close();
}

bool Book::dateTimeExist(const QDateTime &dt) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM Transactions WHERE Date = :d");
    query.bindValue(":d", dt.toString(DATE_TIME_FORMAT));
    query.exec();
    return query.next();
}

Transaction Book::getTransaction(const QDateTime &dt)
{
    Transaction t;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM Transactions WHERE Date = :d");
    query.bindValue(":d", dt.toString(DATE_TIME_FORMAT));
    query.exec();
    if (query.next())
    {
        t.m_dateTime = dt;
        t.m_description = query.value("Description").toString();
        t.stringToData(Account::Expense,   query.value("Expense").toString());
        t.stringToData(Account::Revenue,   query.value("Revenue").toString());
        t.stringToData(Account::Asset,     query.value("Asset").toString());
        t.stringToData(Account::Liability, query.value("Liability").toString());
    }
    return t;
}

bool Book::insertTransaction(const Transaction &t) const
{
    if (!t.validation().isEmpty()) return false;

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO Transactions (Date, Description, Expense, Revenue, Asset, Liability) "
                                "VALUES (:dateTime, :description, :expense, :revenue, :asset, :liability)");
    query.bindValue(":dateTime",    t.m_dateTime.toString(DATE_TIME_FORMAT));
    query.bindValue(":description", t.m_description);
    query.bindValue(":expense",     t.dataToString(Account::Expense));
    query.bindValue(":revenue",     t.dataToString(Account::Revenue));
    query.bindValue(":asset",       t.dataToString(Account::Asset));
    query.bindValue(":liability",   t.dataToString(Account::Liability));

    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << "#Error Insert a transaction:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<Transaction> Book::queryTransactions(const QDateTime &startTime, const QDateTime &endTime, const QString &description, const QList<Account> &accounts) const
{
    QList<Transaction> transactionList;

    QString statement = "";
    for (Account account : accounts)
        if (!account.m_category.isEmpty())
            statement += " AND (" + account.getTableName() + " LIKE \"%[" + account.m_category + "|" + account.m_name + ":%\")";

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM Transactions"
                  " WHERE (Date BETWEEN :startDate AND :endDate)"
                  " AND (Description LIKE :description)" +
                  statement +
                  " ORDER BY Date DESC"); // ASC, DESC

    query.bindValue(":startDate", startTime.toString(DATE_TIME_FORMAT));
    query.bindValue(":endDate",     endTime.toString(DATE_TIME_FORMAT));
    query.bindValue(":description", "%" + description + "%");

    if (!query.exec())
        qDebug() << Q_FUNC_INFO << query.lastError();

    while (query.next()) {
        Transaction t;
        t.m_dateTime              = query.value("Date").toDateTime();
        t.m_description           = query.value("Description").toString();
        t.stringToData(Account::Expense,   query.value("Expense").toString());
        t.stringToData(Account::Revenue,   query.value("Revenue").toString());
        t.stringToData(Account::Asset,     query.value("Asset").toString());
        t.stringToData(Account::Liability, query.value("Liability").toString());

//        if (!t.validation().isEmpty())   // uncomment this line for Validation mode
        transactionList.push_back(t);
    }

    return transactionList;
}

QList<FinancialStat> Book::getSummaryByMonth(const QDateTime &endDateTime) const {
  QSqlQuery query(m_database);
  query.prepare("SELECT * FROM Transactions WHERE Date <= :d ORDER BY Date ASC");
  query.bindValue(":d", endDateTime.toString(DATE_TIME_FORMAT));
  if (!query.exec())
    qDebug() << Q_FUNC_INFO << query.lastError();

  QList<FinancialStat> retSummarys;
  FinancialStat cumSummary;
  QDate month = QDate(getFirstTransactionDateTime().date().year(), getFirstTransactionDateTime().date().month(), 1).addMonths(1);

  while (query.next()) {
    Transaction tempTransaction;
    tempTransaction.m_dateTime = query.value("Date").toDateTime();

    tempTransaction.stringToData(Account::Expense,   query.value("Expense")  .toString());
    tempTransaction.stringToData(Account::Revenue,   query.value("Revenue")  .toString());
    tempTransaction.stringToData(Account::Asset,     query.value("Asset")    .toString());
    tempTransaction.stringToData(Account::Liability, query.value("Liability").toString());

    while (tempTransaction.m_dateTime.date() >= month) {
      cumSummary.m_description = month.addMonths(-1).toString("yyyy-MM");
      retSummarys.push_front(cumSummary);
      month = month.addMonths(1);
      cumSummary.clear(Account::Revenue);
      cumSummary.clear(Account::Expense);
    }

    cumSummary += tempTransaction;
    cumSummary.retainedEarnings += tempTransaction.getRetainedEarnings();
  }
  cumSummary.m_description = month.addMonths(-1).toString("yyyy-MM");
  retSummarys.push_front(cumSummary);

  return retSummarys;
}

void Book::removeTransaction(const QDateTime &dateTime) const
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM Transactions WHERE Date = :d");
    query.bindValue(":d", dateTime.toString(DATE_TIME_FORMAT));

    if (!query.exec())
        qDebug() << Q_FUNC_INFO << query.lastError();
}

bool Book::moveAccount(const Account &account, const Account &newAccount) const {
    if (account.m_table != newAccount.m_table) {
        qDebug() << Q_FUNC_INFO << "Does not support change table yet.";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM Transactions WHERE [" + account.getTableName() + "] LIKE :exp");
    query.bindValue(":exp", "%[" + account.m_category + "|" + account.m_name + ": %");
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return false;
    }
    // Update transactions
    while (query.next())
    {
        QString l_newString = query.value(account.getTableName()).toString()
                .replace("[" + account.m_category + "|" + account.m_name + ": ",
                         "[" +   newAccount.m_category + "|" +   newAccount.m_name + ": ");
        QSqlQuery query2(m_database);
        query2.prepare("UPDATE Transactions SET " + account.getTableName() + " = :new WHERE Date = :d");
        query2.bindValue(":d", query.value("Date").toString());
        query2.bindValue(":new", l_newString);
        if (!query2.exec())
            qDebug() << Q_FUNC_INFO << query2.lastError();
    }
    // Update account
    query.prepare("DELETE FROM " + account.getTableName() + " WHERE Category = :c AND Name = :n");
    query.bindValue(":c", account.m_category);
    query.bindValue(":n", account.m_name);
    query.exec();
    if (!query.exec())
        qDebug() << Q_FUNC_INFO << query.lastError();

    query.prepare("INSERT INTO [" + account.getTableName() + "] (Category, Name) VALUES (:c, :n)");
    query.bindValue(":c", newAccount.m_category);
    query.bindValue(":n", newAccount.m_name);
    query.exec();
    if (!query.exec()) // I don't know why this is keep saying failed but insert correctly
        qDebug() << Q_FUNC_INFO << "Insert new account failed, might be rename the account and merge."
                 << query.lastError() << account.m_category << account.m_name << newAccount.m_category << newAccount.m_name;
    // The reason not use UPDATE directly is considering the merge case

    return true;
}

Currency_e Book::getCurrencyType(const Account &account) const {
    if (account.m_table == Account::Expense or account.m_table == Account::Revenue or account.m_table == Account::Equity)
        return USD;

    QSqlQuery query(m_database);
    query.prepare("SELECT Currency FROM [" + account.getTableName() + "] WHERE Category = :c AND Name = :n");
    query.bindValue(":c", account.m_category);
    query.bindValue(":n", account.m_name);
    if (!query.exec())
        qDebug() << Q_FUNC_INFO << query.lastError();
    if (query.next())
        return Currency::Symbol_3.key(query.value("Currency").toString());
    else {
        qDebug() << Q_FUNC_INFO << "No account was found" << account.getTableName() << account.m_category << account.m_name;
        return USD;
    }
}

QStringList Book::getCategories(const Account::TableType &p_tableType) const {
    QStringList l_categories;
    QSqlQuery l_query(m_database);
    l_query.prepare("SELECT DISTINCT Category FROM [" + Account::TableName.value(p_tableType) + "] ORDER BY Category ASC");
    if (!l_query.exec())
        qDebug() << Q_FUNC_INFO << l_query.lastError();
    while (l_query.next())
    {
        l_categories << l_query.value("Category").toString();
    }
    return l_categories;
}

QStringList Book::getAccountNames(const Account::TableType &p_tableType, const QString &p_category) const
{
    QStringList l_names;
    QSqlQuery l_query(m_database);
    l_query.prepare("SELECT Name FROM [" + Account::TableName.value(p_tableType) + "] WHERE Category = :c ORDER BY Name ASC");
    l_query.bindValue(":c", p_category);

    if (!l_query.exec())
        qDebug() << Q_FUNC_INFO << l_query.lastError();
    while (l_query.next())
    {
        l_names << l_query.value("Name").toString();
    }
    return l_names;
}

QStringList Book::getAccountNamesByLastUpdate(const Account::TableType &tableType, const QString &category, const QDateTime &dateTime) const {
    QMultiMap<QDateTime, QString> accountNamesByDate;
    for (const QString &accountName: getAccountNames(tableType, category))
    {
        QSqlQuery query(m_database);
        query.prepare("SELECT MAX(Date) From Transactions WHERE " + Account::TableName.value(tableType) + " LIKE :account AND Date <= :date");
        query.bindValue(":account", "%[" + category + "|" + accountName + ":%");
        query.bindValue(":date", dateTime.toString(DATE_TIME_FORMAT));
        query.exec();
        if (query.next())
            accountNamesByDate.insertMulti(query.value("MAX(Date)").toDateTime(), accountName);
        else
            accountNamesByDate.insertMulti(QDateTime(QDate(1990, 05, 25)), accountName);
    }

    QStringList accountNames;
    for (const QString &accountName: accountNamesByDate.values())
        accountNames.push_front(accountName);  // This is reverse the list
    return accountNames;
}

bool Book::insertCategory(const QString &p_tableName, const QString &p_category) const {
    QSqlQuery l_query(m_database);
    l_query.prepare("SELECT * FROM [" + p_tableName + "] WHERE Category = :c");   // Maybe build check category exist into a function?
    l_query.bindValue(":c", p_category);
    if (!l_query.exec())
    {
        qDebug() << Q_FUNC_INFO << l_query.lastError();
        return false;
    }
    if (l_query.next())
    {
        qDebug() << Q_FUNC_INFO << p_category << " already exist";
        return false;
    }
    l_query.prepare("INSERT INTO [" + p_tableName + "] (Category, Name) VALUES (:c, :c)");
    l_query.bindValue(":c", p_category);

    if (!l_query.exec())
    {
        qDebug() << Q_FUNC_INFO << l_query.lastError();
        return false;
    }
    else
        return true;
}

bool Book::removeCategory(const QString &tableName, const QString &category) const
{
    QSqlQuery query(m_database);
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
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM [" + tableName + "] WHERE Category = :c");
    query.bindValue(":c", category);
    query.exec();
    return query.next();
}

bool Book::renameCategory(const QString &tableName, const QString &category, const QString &newCategory) const
{
    if (categoryExist(tableName, newCategory))
        return false;

    QSqlQuery query(m_database);
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
        QSqlQuery query2(m_database);
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

bool Book::insertAccount(const Account &account) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM [" + account.getTableName() + "] WHERE Category = :c");  // Maybe build check category exist into a function?
    query.bindValue(":c", account.m_category);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << 1 << query.lastError();
        return false;
    }
    if (!query.next())
    {
        qDebug() << Q_FUNC_INFO << "Category " << account.m_category << " does not exist";
        return false;
    }

    query.prepare("INSERT INTO [" + account.getTableName() + "] (Category, Name) VALUES (:c, :n)");
    query.bindValue(":c", account.m_category);
    query.bindValue(":n", account.m_name);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << 2 << query.lastError();
        return false;
    }

    return true;
}

bool Book::removeAccount(const Account &account) const
{
    QSqlQuery l_query(m_database);
    l_query.prepare("SELECT * FROM Transactions WHERE [" + account.getTableName() + "] LIKE :exp");
    l_query.bindValue(":exp", "%[" + account.m_category + "|" + account.m_name + ":%");
    if (!l_query.exec())
    {
        qDebug() << Q_FUNC_INFO << l_query.lastError();
        return false;
    }
    if (l_query.next())
        return false;
    else
    {
        l_query.prepare("DELETE FROM [" + account.getTableName() + "] WHERE Category = :c AND Name = :n");
        l_query.bindValue(":c", account.m_category);
        l_query.bindValue(":n", account.m_name);
        return l_query.exec();
    }
}

bool Book::accountExist(const Account &account) const
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM [" + account.getTableName() + "] WHERE Category = :c AND Name = :n");
    query.bindValue(":c", account.m_category);
    query.bindValue(":n", account.m_name);
    if (!query.exec())
    {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return false;
    }
    return query.next();
}

QDateTime Book::getFirstTransactionDateTime() const
{
    QDateTime dateTime;
    QSqlQuery query("SELECT MIN(Date) FROM Transactions", m_database);
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
    QSqlQuery query("SELECT MAX(Date) FROM Transactions", m_database);
    if (query.next()) {
        dateTime = query.value("MAX(Date)").toDateTime();
        if (!dateTime.isValid())
            dateTime = QDateTime::currentDateTime();
    }
    return dateTime;
}

void Book::logStartTime()
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO TimeLogging (Start, End) VALUES (:s, NULL)");
    query.bindValue(":s", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
    query.exec();
}

void Book::logEndTime()
{
    QSqlQuery query("SELECT MAX(Start) FROM TimeLogging", m_database);
    QString s;
    if (query.next())
        s = query.value("MAX(Start)").toString();

    query.prepare("UPDATE TimeLogging SET End = :e WHERE Start = :s");
    query.bindValue(":e", QDateTime::currentDateTime().toString(DATE_TIME_FORMAT));
    query.bindValue(":s", s);
    query.exec();
}
