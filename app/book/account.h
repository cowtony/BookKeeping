#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QString>
#include <QSharedPointer>
#include <QMap>

#include "currency/currency.h"

class Account {
public:
    typedef enum {Asset, Liability, Revenue, Expense, Equity, INVALID} Type;
    static const QMap<Type, QString> kAccountTypeName;

    explicit Account(int account_id, int category_id, const QString& category_name, const QString& account_name, const QString& comment = "")
        : account_id_(account_id), category_id_(category_id), category_name_(category_name), account_name_(account_name), comment_(comment) {}

    // Factory Creating Method
    static QSharedPointer<Account> create(int account_id,
                                          int category_id,
                                          Account::Type account_type,
                                          const QString& category_name,
                                          const QString& account_name,
                                          const QString& comment = "",
                                          Currency::Type currency_type = Currency::USD,
                                          bool is_investment = false);

    // Setters:
    void setComment(const QString& comment) { comment_ = comment; }
    virtual void setIsInvestment(bool /*is_investment*/) {}

    // Getters:
    int     accountId() const { return account_id_; }
    int     categoryId() const { return category_id_; }
    QString typeName() const { return kAccountTypeName.value(accountType()); }
    QString categoryName() const { return category_name_; }
    QString accountName() const { return account_name_; }
    QString comment() const { return comment_; }
    virtual Currency::Type currencyType() const { return Currency::USD; }
    virtual Type accountType() const = 0;
    virtual bool isInvestment() const { return false; }
    virtual QString getFinancialStatementName() const = 0; // Income Statement, Balance Sheet, TODO: add Cash Flow

private:
    int     account_id_;
    int     category_id_;
    QString category_name_;
    QString account_name_;
    QString comment_;
};


class BalanceSheetAccount : public Account {
public:
    Currency::Type currencyType() const override { return currency_type_; }
    QString getFinancialStatementName() const override { return "Balance Sheet"; };

protected:
    explicit BalanceSheetAccount(int account_id, int category_id, const QString& category_name, const QString& account_name, const QString& comment, Currency::Type currency_type)
        : Account(account_id, category_id, category_name, account_name, comment), currency_type_(currency_type) {}

private:
    Currency::Type currency_type_;
};


class IncomeStatementAccount : public Account {
public:
    QString getFinancialStatementName() const override { return "Income Statement"; };

protected:
    explicit IncomeStatementAccount(int account_id, int category_id, const QString& category_name, const QString& account_name, const QString& comment)
        : Account(account_id, category_id, category_name, account_name, comment) {}
};


class AssetAccount final : public BalanceSheetAccount {
public:
    explicit AssetAccount(int account_id, int category_id, const QString& category_name, const QString& account_name, const QString& comment, Currency::Type currency_type, bool is_investment)
        : BalanceSheetAccount(account_id, category_id, category_name, account_name, comment, currency_type), is_investment_(is_investment) {}

    Type accountType() const override { return Asset; }
    bool isInvestment() const override { return is_investment_; }
    void setIsInvestment(bool is_investment) override { is_investment_ = is_investment; }

private:
    bool is_investment_;
};


class LiabilityAccount final : public BalanceSheetAccount {
public:
    explicit LiabilityAccount(int account_id, int category_id, const QString& category_name, const QString& account_name, const QString& comment, Currency::Type currency_type)
        : BalanceSheetAccount(account_id, category_id, category_name, account_name, comment, currency_type) {}

    Type accountType() const override { return Liability; }

private:
};


class EquityAccount final : public BalanceSheetAccount {
public:
    explicit EquityAccount(int account_id, int category_id, const QString& category_name, const QString& account_name, const QString& comment, Currency::Type currency_type)
        : BalanceSheetAccount(account_id, category_id, category_name, account_name, comment, currency_type) {}

    Type accountType() const override { return Equity; }

private:
};


class ExpenseAccount final : public IncomeStatementAccount {
public:
    explicit ExpenseAccount(int account_id, int category_id, const QString& category_name, const QString& account_name, const QString& comment)
        : IncomeStatementAccount(account_id, category_id, category_name, account_name, comment) {}

    Type accountType() const override { return Expense; }

private:
};


class RevenueAccount final : public IncomeStatementAccount {
public:
    explicit RevenueAccount(int account_id, int category_id, const QString& category_name, const QString& account_name, const QString& comment)
        : IncomeStatementAccount(account_id, category_id, category_name, account_name, comment) {}

    Type accountType() const override { return Revenue; }

private:
};

#endif // ACCOUNT_H
