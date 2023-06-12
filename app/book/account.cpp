#include "account.h"

#include <QDebug>

const QMap<Account::Type, QString> Account::kAccountTypeName =
    {{Asset, "Asset"},
     {Liability, "Liability"},
     {Revenue, "Revenue"},
     {Expense, "Expense"},
     {Equity, "Equity"}};


QSharedPointer<Account> Account::create(int account_id,
                                        int category_id,
                                        Account::Type account_type,
                                        const QString& category_name,
                                        const QString& account_name,
                                        const QString& comment,
                                        Currency::Type currency_type,
                                        bool is_investment) {
    switch (account_type) {
        case Account::Asset:
            return QSharedPointer<AssetAccount>    (new     AssetAccount(account_id, category_id, category_name, account_name, comment, currency_type, is_investment));
        case Account::Liability:
            return QSharedPointer<LiabilityAccount>(new LiabilityAccount(account_id, category_id, category_name, account_name, comment, currency_type));
        case Account::Equity:
            return QSharedPointer<EquityAccount>   (new    EquityAccount(account_id, category_id, category_name, account_name, comment, currency_type));
        case Account::Expense:
            return QSharedPointer<ExpenseAccount>  (new   ExpenseAccount(account_id, category_id, category_name, account_name, comment));
        case Account::Revenue:
            return QSharedPointer<RevenueAccount>  (new   RevenueAccount(account_id, category_id, category_name, account_name, comment));
        default:
            qDebug() << "\e[0;31m" << __FILE__ << "line" << __LINE__ << Q_FUNC_INFO << ":\e[0m";
            return nullptr;
    }
}
