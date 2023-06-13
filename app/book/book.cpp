#include "book.h"

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
        Q_ASSERT(false);
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

bool Book::insertTransaction(int user_id, const Transaction& transaction, bool ignore_error) {
    if (!ignore_error && !transaction.validate().isEmpty()) {
        return false;
    }

    db.transaction();
    QSqlQuery query(db);

    query.prepare(R"sql(INSERT INTO book_transactions (user_id, date_time, description) VALUES (:user_id, :dateTime, :description))sql");
    query.bindValue(":user_id",     user_id);
    query.bindValue(":dateTime",    transaction.date_time.toString(kDateTimeFormat));
    query.bindValue(":description", transaction.description);

    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        db.rollback();
        return false;
    }
    Logging(query);

    query.prepare(R"sql(INSERT INTO book_transaction_details (transaction_id, account_id, household_id, currency_id, amount)
                        VALUES (
                            :transaction_id,
                            (SELECT account_id FROM accounts_view WHERE user_id = :user_id AND type_name = :type_name AND category_name = :cat AND account_name = :acc),
                            (SELECT household_id FROM book_households WHERE user_id = :user_id AND name = :household_name),
                            (SELECT currency_id FROM currency_types WHERE Name = :currency_name),
                            :amount
                        ) )sql");
    query.bindValue(":transaction_id", query.lastInsertId().toInt());
    query.bindValue(":user_id", user_id);

    for (const auto& [account, household_money] : transaction.getAccounts()) {
        query.bindValue(":type_name", account->typeName());
        query.bindValue(":cat", account->categoryName());
        query.bindValue(":acc", account->accountName());
        for (const auto& [household, money] : household_money.asKeyValueRange()) {
            if (money.isZero()) {
                continue;
            }
            query.bindValue(":household_name", household);
            query.bindValue(":currency_name", Currency::kCurrencyToCode.value(money.currency()));
            query.bindValue(":amount", money.amount_);
            if (!query.exec()) {
                qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
                db.rollback();
                return false;
            }
            Logging(query);
        }
    }

    if (!db.commit()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << db.lastError();
        db.rollback();
        return false;
    }
    return true;
}

QList<Transaction> Book::queryTransactions(int user_id, const TransactionFilter& filter) const {
    QStringList statements;
    for (const auto& [account, household_money] : filter.getAccounts()) {
        if (!account->categoryName().isEmpty()) {
            statements << QString(R"sql((%1 LIKE '%%2|%3%'))sql").arg(account->typeName(), account->categoryName(), account->accountName());
        }
    }

    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT *
                                FROM   transaction_details_view
                                WHERE  transaction_id IN (
                                    SELECT DISTINCT transaction_id
                                    FROM (
                                        SELECT   transaction_id, Timestamp
                                        FROM     transactions_view
                                        WHERE    user_id = :user
                                                 AND Timestamp BETWEEN :start AND :end
                                                 AND Description LIKE :description
                                                 AND (%1)
                                )
                                ORDER BY Timestamp %2, transaction_id %2
                                LIMIT    :limit
                            ) )sql").arg(statements.empty()? "TRUE" : statements.join(filter.use_or? " OR " : " AND "),
                           filter.ascending_order? "ASC" : "DESC"));
    query.bindValue(":user", user_id);
    query.bindValue(":start", filter.date_time.toString(kDateTimeFormat));
    query.bindValue(":end",   filter.end_date_time.toString(kDateTimeFormat));
    query.bindValue(":description", "%" + filter.description + "%");
    query.bindValue(":limit", filter.limit);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return {};
    }

    QList<Transaction> result;
    int current_transaction_id = -1;
    Transaction transaction;
    while (query.next()) {
        if (query.value("transaction_id").toInt() != current_transaction_id) { // New transaction.
            if (current_transaction_id > 0) {
                result.push_back(transaction);
            }
            transaction.clear();
            current_transaction_id = query.value("transaction_id").toInt();
            transaction.id = current_transaction_id;
            transaction.description = query.value("description").toString();
            transaction.date_time = query.value("date_time").toDateTime();
        }
        auto account = Account::create(query.value("account_id").toInt(),
                                       query.value("category_id").toInt(),
                                       Account::kAccountTypeName.key(query.value("type_name").toString()),
                                       query.value("category_name").toString(),
                                       query.value("account_name").toString(),
                                       "",
                                       Currency::kCurrencyToSymbol.key(query.value("currency_symbol").toString()));
        Money money(transaction.date_time.date(),
                    Currency::kCurrencyToSymbol.key(query.value("currency_symbol").toString()),
                    query.value("amount").toDouble());
        transaction.addMoney(account, query.value("household_name").toString(), money);
    }
    if (current_transaction_id > 0) {
        result.push_back(transaction);
    }
    qDebug() << result.size();
    return result;
}

Transaction Book::getTransaction(int transaction_id) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT * FROM transaction_details_view WHERE transaction_id = :id)sql");
    query.bindValue(":id", transaction_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return Transaction();
    }
    Transaction transaction;
    transaction.id = transaction_id;
    while (query.next()) {
        transaction.description = query.value("description").toString();
        transaction.date_time = query.value("date_time").toDateTime();
        auto account = Account::create(query.value("account_id").toInt(),
                                       query.value("category_id").toInt(),
                                       Account::kAccountTypeName.key(query.value("type_name").toString()),
                                       query.value("category_name").toString(),
                                       query.value("account_name").toString(),
                                       "",
                                       Currency::kCurrencyToSymbol.key(query.value("currency_symbol").toString()));
        Money money(transaction.date_time.date(),
                    Currency::kCurrencyToSymbol.key(query.value("currency_symbol").toString()),
                    query.value("amount").toDouble());
        transaction.addMoney(account, query.value("household_name").toString(), money);
    }
    return transaction;
}

void Book::removeTransaction(int transaction_id) {
    db.transaction();
    QSqlQuery query(db);
    query.prepare(R"sql(DELETE FROM book_transactions WHERE transaction_id = :id)sql");
    query.bindValue(":id", transaction_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        db.rollback();
        return;
    }
    query.prepare(R"sql(DELETE FROM book_transaction_details WHERE transaction_id = :id)sql");
    query.bindValue(":id", transaction_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        db.rollback();
        return;
    }

    if (!db.commit()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << db.lastError();
        db.rollback();
    }
    Logging(query);
}

// TODO: This need to be changed after transactions table has been migrated.
QString Book::moveAccount(int user_id, const Account& old_account, const Account& new_account) {
    if (old_account.accountType() != new_account.accountType()) {
        return "Does not support move account between different account type yet.";
    }

    QSqlQuery query(db);
    query.prepare(QString(R"sql(SELECT transaction_id, detail->'%1' AS type_detail
                                FROM book_transactions
                                WHERE type_detail LIKE :expression)sql").arg(old_account.typeName()));
    query.bindValue(":expression", QString(R"(%"%1|%2":%)").arg(old_account.categoryName(), old_account.accountName()));
    if (!query.exec()) {
        qDebug() << Q_FUNC_INFO << query.lastError();
        return "Error execute query.";
    }
    // Update transactions
    while (query.next()) {
        QString new_string = query.value("type_detail").toString().replace(QString(R"("%1|%2":)").arg(old_account.categoryName(), old_account.accountName()),
                                                                           QString(R"("%1|%2":)").arg(new_account.categoryName(), new_account.accountName()));
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
    if (old_account.accountType() == Account::Asset && IsInvestment(user_id, old_account)) {
        query.prepare(R"sql(SELECT transaction_id, detail->'Revenue' AS revenue_detail
                            FROM book_transactions
                            WHERE revenue_detail LIKE :expression)sql");
        query.bindValue(":expression", QString(R"(%"Investment|%1":%)").arg(old_account.accountName()));
        if (!query.exec()) {
            qDebug() << Q_FUNC_INFO << query.lastError();
            return "Error execute query.";
        }
        while (query.next()) {
            QString new_string = query.value("revenue_detail").toString().replace(QString(R"("Investment|%1":)").arg(old_account.accountName()),
                                                                                  QString(R"("Investment|%1":)").arg(new_account.accountName()));
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
    if (accountExist(user_id, new_account)) {  // Merge account
        // TODO: old comment is ignored.
        query.prepare(QString(R"sql(DELETE FROM [%1] WHERE Category = :c AND Name = :n)sql").arg(old_account.typeName()));
        query.bindValue(":oc", old_account.categoryName());
        query.bindValue(":on", old_account.accountName());
        query.exec();
        // TODO: Investment force set to true.
        if (new_account.accountType() == Account::Asset) {
            return setInvestment(user_id, static_cast<const AssetAccount&>(new_account), true);
        }
    } else {  // Rename account
        query.prepare(QString(R"sql(UPDATE %1 SET Category = :nc, Name = :nn WHERE Category = :oc AND Name = :on)sql").arg(old_account.typeName()));
        query.bindValue(":nc", new_account.categoryName());
        query.bindValue(":nn", new_account.accountName());
        query.bindValue(":oc", old_account.categoryName());
        query.bindValue(":on", old_account.accountName());
        if (query.exec()) {
            Logging(query);
        } else {
            qDebug() << Q_FUNC_INFO << query.lastError();
            return "Error execute query.";
        }
    }

    return "";  // OK status.
}

QStringList Book::getHouseholds(int user_id) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT user_id, name
                        FROM   book_households
                        WHERE  user_id = :user
                        ORDER BY rank ASC)sql");
    query.bindValue(":user", user_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return {};
    }
    QStringList households;
    while (query.next()) {
        households << query.value("name").toString();
    }
    return households;
}

Currency::Type Book::queryCurrencyType(int user_id, Account::Type account_type, const QString& category_name, const QString& account_name) const {
    if (account_type == Account::Expense || account_type == Account::Revenue || account_type == Account::Equity) {
        return Currency::USD;  // Only Asset and Liability allows different currency.
    }

    QSqlQuery query(db);
    query.prepare(R"sql(SELECT currency_name
                        FROM   accounts_view
                        WHERE  user_id = :user AND type_name = :type AND category_name = :cat AND account_name = :acc)sql");
    query.bindValue(":user", user_id);
    query.bindValue(":type", Account::kAccountTypeName.value(account_type));
    query.bindValue(":cat", category_name);
    query.bindValue(":acc", account_name);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
    }
    if (!query.next()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m"
                 << "No account was found: " << Account::kAccountTypeName.value(account_type) << category_name << account_name;
        return Currency::USD;
    }
    return Currency::kCurrencyToCode.key(query.value("currency_name").toString());
}

QStringList Book::queryCategories(int user_id, Account::Type account_type) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT c.category_name AS category_name
                        FROM book_account_categories AS c
                        JOIN book_account_types      AS t ON c.account_type_id = t.account_type_id
                        WHERE c.user_id = :user AND t.type_name = :type
                        ORDER BY category_name ASC)sql");
    query.bindValue(":user", user_id);
    query.bindValue(":type", Account::kAccountTypeName.value(account_type));
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
    }
    QStringList categories;
    while (query.next()) {
        categories << query.value("category_name").toString();
    }
    if (account_type == Account::Revenue) {
        categories << "Investment";
    }
    return categories;
}

QStringList Book::queryAccounts(int user_id, Account::Type account_type, const QString& category) const {
    QStringList accounts;

    // Special treatment for retriving Revenue::Investment.
    if (account_type == Account::Revenue and category == "Investment") {
        for (const AssetAccount& investment : getInvestmentAccounts(user_id)) {
            accounts << investment.accountName();
        }
        return accounts;
    }

    QSqlQuery query(db);
    query.prepare(R"sql(SELECT   account_name
                        FROM     accounts_view
                        WHERE    user_id = :user AND type_name = :type AND category_name = :cat
                        ORDER BY account_name ASC)sql");
    query.bindValue(":user", user_id);
    query.bindValue(":type", Account::kAccountTypeName.value(account_type));
    query.bindValue(":cat", category);

    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return {};
    }
    while (query.next()) {
        accounts << query.value("account_name").toString();
    }
    return accounts;
}

QList<AssetAccount> Book::getInvestmentAccounts(int user_id) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT   *
                        FROM     accounts_view
                        WHERE    user_id = :user AND account_type_id = 1 AND is_investment = True
                        ORDER BY account_name ASC)sql");
    query.bindValue(":user", user_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return {};
    }
    QList<AssetAccount> investments;
    while (query.next()) {
        investments << AssetAccount(query.value("account_id").toInt(),
                                    query.value("category_id").toInt(),
                                    query.value("category_name").toString(),
                                    query.value("account_name").toString(),
                                    query.value("comment").toString(),
                                    Currency::kCurrencyToCode.key(query.value("currency_name").toString()),
                                    true);
    }
    return investments;
}

QList<QSharedPointer<Account>> Book::queryAllCategories(int user_id) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT   category_id, category_name, type_name
                        FROM     book_account_categories AS c
                        JOIN     book_account_types      AS t ON c.account_type_id = t.account_type_id
                        WHERE    user_id = :user AND c.account_type_id IN (1, 2, 3, 4)
                        ORDER BY category_name ASC)sql");
    query.bindValue(":user", user_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return {};
    }

    QList<QSharedPointer<Account>> categories;
    while (query.next()) {
        QSharedPointer<Account> category = Account::create(-1,
                                                           query.value("category_id").toInt(),
                                                           Account::kAccountTypeName.key(query.value("type_name").toString()),
                                                           query.value("category_name").toString(),
                                                           /*account_name=*/"");
        categories << category;
    }
    return categories;
}

QList<QSharedPointer<Account>> Book::queryAllAccounts(int user_id) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT   *
                        FROM     accounts_view
                        WHERE    user_id = :user AND account_type_id IN (1, 2, 3, 4)
                        ORDER BY category_name ASC, account_name ASC)sql");
    query.bindValue(":user", user_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return {};
    }

    QList<QSharedPointer<Account>> accounts;
    while (query.next()) {
        QSharedPointer<Account> account = Account::create(query.value("account_id").toInt(),
                                                          query.value("category_id").toInt(),
                                                          Account::kAccountTypeName.key(query.value("type_name").toString()),
                                                          query.value("category_name").toString(),
                                                          query.value("account_name").toString(),
                                                          query.value("comment").toString(),
                                                          Currency::kCurrencyToCode.key(query.value("currency_name").toString()),
                                                          query.value("is_investment").toBool());
        accounts << account;
    }
    return accounts;
}

QList<QSharedPointer<Account>> Book::queryAccountNamesByLastUpdate(int user_id, Account::Type account_type, const QString& category_name, const QDateTime& date_time) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT    a.account_id, d.category_id, a.account_name, a.comment, d.currency_symbol, a.is_investment, MAX(d.date_time) AS max_date_time
                        FROM      book_accounts AS a
                        LEFT JOIN transaction_details_view AS d
                               ON a.account_id = d.account_id AND
                                  d.user_id = :user_id AND
                                  d.type_name = :type_name AND
                                  d.date_time < :date_time
                        WHERE     a.category_id = (SELECT category_id FROM transaction_details_view WHERE category_name = :category_name LIMIT 1)
                        GROUP BY  a.account_id
                        ORDER BY  max_date_time DESC)sql");
    query.bindValue(":user_id", user_id);
    query.bindValue(":type_name", Account::kAccountTypeName.value(account_type));
    query.bindValue(":date_time", date_time);
    query.bindValue(":category_name", category_name);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return {};
    }
    QList<QSharedPointer<Account>> result;
    while (query.next()) {
        result << Account::create(query.value("account_id").toInt(),
                                  query.value("category_id").toInt(),
                                  account_type,
                                  category_name,
                                  query.value("account_name").toString(),
                                  query.value("comment").toString(),
                                  Currency::kCurrencyToSymbol.key(query.value("currency_symbol").toString()),
                                  query.value("is_investment").toBool());
    }
    return result;
}

bool Book::updateAccountComment(int account_id, const QString& comment) const {
    QSqlQuery query(db);
    query.prepare(R"sql(UPDATE book_accounts
                        SET comment = :comment
                        WHERE account_id = :account_id)sql");
    query.bindValue(":account_id", account_id);
    query.bindValue(":comment", comment);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return false;
    }
    return true;
}

QString Book::setInvestment(int user_id, const AssetAccount& asset, bool is_investment) {
    if (!is_investment) {  // Set to false
        auto investment_account = getAccount(user_id, Account::Revenue, "Investment", asset.accountName());
        if (investment_account) {
            QSqlQuery query(db);
            // Check for any transacion still associate with this Investment account.
            query.prepare(R"sql(SELECT * FROM book_transaction_details WHERE account_id = :account_id LIMIT 1)sql");
            query.bindValue(":account_id", investment_account->accountId());
            if (!query.exec()) {
                qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
                return "ERROR: " + query.lastError().text();
            }
            if (query.next()) {
                return "Error: Cannot remove " + asset.accountName() + " from investment since there still are transactions associate with it.";
            }

            db.transaction();
            query.prepare(R"sql(UPDATE book_accounts SET is_investment = False WHERE account_id = :account_id)sql");
            query.bindValue(":account_id", asset.accountId());
            if (!query.exec()) {
                qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
                db.rollback();
                return "ERROR: " + query.lastError().text();
            }
            query.prepare(R"sql(DELETE FROM book_accounts WHERE account_id = :account_id)sql");
            query.bindValue(":account_id", investment_account->accountId());
            if (!query.exec()) {
                qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
                db.rollback();
                return "ERROR: " + query.lastError().text();
            }
            if (!db.commit()) {
                qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << db.lastError();
                db.rollback();
                return "ERROR: " + db.lastError().text();
            }
        }

    } else { // Set to true
        db.transaction();
        QSqlQuery query(db);
        query.prepare(R"sql(UPDATE book_accounts SET is_investment = True WHERE account_id = :account_id)sql");
        query.bindValue(":account_id", asset.accountId());
        if (!query.exec()) {
            qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
            db.rollback();
            return "ERROR: " + query.lastError().text();
        }
        // Cannot call `insertAccount()` for this because we want it in a transaction, also `insertAccount()` prevent manipulation of Investment category.
        query.prepare(R"sql(INSERT INTO book_accounts (category_id, account_name, comment)
                            VALUES (
                                (SELECT category_id FROM book_account_categories WHERE user_id = :user_id AND account_type_id = 3 AND category_name = 'Investment' LIMIT 1),
                                :account_name,
                                'Auto generated account'
                            ) )sql");
        query.bindValue(":user_id", user_id);
        query.bindValue(":account_name", asset.accountName());
        if (!query.exec()) {
            qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
            db.rollback();
            return "ERROR: " + query.lastError().text();
        }
        if (!db.commit()) {
            qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << db.lastError();
            db.rollback();
            return "ERROR: " + db.lastError().text();
        }
    }
    return "";  // Ok status
}

bool Book::IsInvestment(int user_id, const Account& account) const {
    if (account.accountType() != Account::Asset) {
        return false;  // Only Asset account can be defined as investment.
    }

    QSqlQuery query(db);
    query.prepare(R"sql(SELECT is_investment
                        FROM   accounts_view
                        WHERE  user_id = :user AND account_type_id = 1 AND category_name = :cat AND account_name = :acc)sql");
    query.bindValue(":user", user_id);
    query.bindValue(":cat", account.categoryName());
    query.bindValue(":acc", account.accountName());
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return false;
    }
    if (query.next()) {
        return query.value("is_investment").toBool();
    }
    return false;
}

bool Book::renameCategory(int user_id, Account::Type account_type, const QString& category_name, const QString& new_category_name) const {
    if (account_type == Account::Revenue && category_name == "Investment") {
        return false; // Cannot manipulate Investment category.
    }
    if (new_category_name.isEmpty() || getCategory(user_id, account_type, new_category_name)) {
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"sql(UPDATE book_account_categories
                        SET    category_name = :new_category_name
                        WHERE  category_id IN (
                            SELECT category_id
                            FROM   accounts_view
                            WHERE  user_id = :user_id AND type_name = :type_name AND category_name = :old_category_name
                        ) )sql");
    query.bindValue(":user_id", user_id);
    query.bindValue(":type_name", Account::kAccountTypeName.value(account_type));
    query.bindValue(":old_category_name", category_name);
    query.bindValue(":new_category_name", new_category_name);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return false;
    }
    return true;
}

QSharedPointer<Account> Book::getCategory(int user_id, Account::Type account_type, const QString &category_name) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT *
                        FROM   book_account_categories AS c
                        JOIN   book_account_types      AS t ON c.account_type_id = t.account_type_id
                        WHERE  user_id = :user_id AND type_name = :type_name AND category_name = :category_name)sql");
    query.bindValue(":user_id", user_id);
    query.bindValue(":type_name", Account::kAccountTypeName.value(account_type));
    query.bindValue(":category_name", category_name);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return nullptr;
    }
    if (!query.next()) {
        return nullptr;  // Category not found.
    }
    return Account::create(-1, query.value("category_id").toInt(), account_type, category_name, "");
}

QSharedPointer<Account> Book::getAccount(int user_id, Account::Type account_type, const QString &category_name, const QString &account_name) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT *
                        FROM   accounts_view
                        WHERE  user_id = :user_id AND type_name = :type_name AND category_name = :category_name AND account_name = :account_name)sql");
    query.bindValue(":user_id", user_id);
    query.bindValue(":type_name", Account::kAccountTypeName.value(account_type));
    query.bindValue(":category_name", category_name);
    query.bindValue(":account_name", account_name);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return nullptr;
    }
    if (!query.next()) {
        return nullptr;  // Account not found.
    }
    return Account::create(query.value("account_id").toInt(),
                           query.value("category_id").toInt(),
                           account_type,
                           category_name,
                           account_name,
                           query.value("comment").toString(),
                           Currency::kCurrencyToCode.key(query.value("currency_name").toString()),
                           query.value("is_investment").toBool());
}

QSharedPointer<Account> Book::insertCategory(int user_id, Account::Type account_type, const QString& category_name) const {
    QSharedPointer<Account> category = getCategory(user_id, account_type, category_name);
    if (category) {
        return category;  // Category exist.
    }

    QSqlQuery query(db);
    query.prepare(R"sql(INSERT INTO book_account_categories (user_id, account_type_id, category_name)
                        VALUES (
                            :user_id,
                            (SELECT account_type_id FROM book_account_types WHERE type_name = :type_name),
                            :category_name
                        ) )sql");
    query.bindValue(":user_id", user_id);
    query.bindValue(":type_name", Account::kAccountTypeName.value(account_type));
    query.bindValue(":category_name", category_name);

    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return nullptr;
    }
    return Account::create(-1, query.lastInsertId().toInt(), account_type, category_name, "");
}

QSharedPointer<Account> Book::insertAccount(int user_id, Account::Type account_type, const QString& category_name, const QString& account_name) const {
    if (account_type == Account::Revenue && category_name == "Investment") {
        return nullptr;  // Investment is a auto managed category, cannot insert account there.
    }

    QSharedPointer<Account> category = getCategory(user_id, account_type, category_name);
    if (!category) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << "Category '" << category_name << "' does not exist!";
        return nullptr;
    }

    QSqlQuery query(db);
    query.prepare(R"sql(INSERT INTO book_accounts (category_id, account_name)
                        VALUES (:category_id, :account_name) )sql");
    query.bindValue(":category_id", category->categoryId());
    query.bindValue(":account_name", account_name);
    if (!query.exec()) {
        // This might because the account_name already exist.
        // TODO: If exist, should we return the existing account?
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return nullptr;
    }
    return Account::create(query.lastInsertId().toInt(), category->categoryId(), account_type, category_name, account_name);
}

bool Book::removeCategory(int category_id) const {
    // TODO: add check to prevent remove category Revenue::Investment.
    // Check if the category still has accounts associated with it.
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT * FROM book_accounts WHERE category_id = :category_id LIMIT 1)sql");
    query.bindValue(":category_id", category_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return false;
    }
    if (query.next()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << "Has account still under this category.";
        return false;
    }

    // Remove this category:
    query.prepare(R"sql(DELETE FROM book_account_categories WHERE category_id = :category_id)sql");
    query.bindValue(":category_id", category_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return false;
    }
    return true;  // TODO: if category doesn't exist, return true or false?
}

bool Book::removeAccount(int account_id) const {
    // TODO: add check to prevent remove account from Revenue::Investment.

    // Check if the account still has transactions associated with it.
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT * FROM book_transaction_details WHERE account_id = :account_id LIMIT 1)sql");
    query.bindValue(":account_id", account_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return false;
    }
    if (query.next()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << "Has transactions still related with this account.";
        return false;
    }

    // TODO: What if removing an account that still "IsInvestment"?

    // Remove this account:
    query.prepare(R"sql(DELETE FROM book_accounts WHERE account_id = :account_id)sql");
    query.bindValue(":account_id", account_id);
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return false;
    }
    return true;  // TODO: if account doesn't exist, return true or false?
}

bool Book::accountExist(int user_id, const Account& account) const {
    QSqlQuery query(db);
    query.prepare(R"sql(SELECT *
                        FROM   accounts_view
                        WHERE  user_id = :user AND type_name = :type AND category_name = :cat AND account_name = :acc)sql");
    query.bindValue(":user", user_id);
    query.bindValue(":type", account.typeName());
    query.bindValue(":cat", account.categoryName());
    query.bindValue(":acc", account.accountName());
    if (!query.exec()) {
        qDebug() << "\e[0;32m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m" << query.lastError();
        return false;
    }
    return query.next();
}

QDateTime Book::getFirstTransactionDateTime() const {
    QDateTime dateTime;
    QSqlQuery query(R"sql(SELECT MIN(date_time) FROM book_transactions)sql", db);
    if (query.next()) {
        dateTime = query.value("MIN(date_time)").toDateTime();
        if (!dateTime.isValid()) {
            dateTime = QDateTime::currentDateTime();
        }
    }
    return dateTime;
}

QDateTime Book::getLastTransactionDateTime() const
{
    QDateTime dateTime;
    QSqlQuery query(R"sql(SELECT MAX(date_time) FROM book_transactions)sql", db);
    if (query.next()) {
        dateTime = query.value("MAX(date_time)").toDateTime();
        if (!dateTime.isValid()) {
            dateTime = QDateTime::currentDateTime();
        }
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
