#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QString>
#include <QMap>

struct Account {
    typedef enum {Asset, Liability, Revenue, Expense, Equity} Type;
    static const QMap<Type, QString> kTableName;

    explicit Account(Type account_type, const QString& category, const QString& name, const QString& comment = "")
                   : type(account_type), category(category), name(name), comment(comment) {}
    explicit Account(const QString& account_type, const QString& category, const QString& name, const QString& comment = "");

    Type    type;
    QString category;
    QString name;
    QString comment;

    QString typeName() const;
    QString getFinancialStatementName() const; // Income Statement, Balance Sheet, Cash Flow

    bool operator  <(const Account& account) const;
    bool operator ==(const Account& account) const;

  private:
    // int id_;  // Primary key in database, not used.
};

struct AssetAccount : public Account {
    AssetAccount(Account account) : Account(account.type, account.category, account.name, account.comment), is_investment(false) {}

    explicit AssetAccount(Type account_type, const QString& category, const QString& name, const QString& comment = "", bool is_investment = false)
                : Account(account_type, category, name, comment), is_investment(is_investment) {}

    bool is_investment;
};

std::shared_ptr<Account> FactoryCreateAccount(Account::Type account_type, const QString& category, const QString& name, const QString& comment = "");

#endif // ACCOUNT_H
