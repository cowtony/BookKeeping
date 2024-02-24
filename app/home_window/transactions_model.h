#ifndef TRANSACTIONS_MODEL_H
#define TRANSACTIONS_MODEL_H

#include <QSqlQueryModel>

#include "book/book.h"
#include "book/transaction.h"

const QVector<Account::Type> kAccountTypes = {Account::Expense, Account::Revenue, Account::Asset, Account::Liability};

class TransactionsModel : public QSqlQueryModel {
    Q_OBJECT

  public:
    explicit TransactionsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QString getDisplayRoleText(int row, int col) const;
    Transaction getTransaction(int row);
    void setFilter(const TransactionFilter& filter);

    const static int kTransactionIdColumnIndex = 6;
    const static int kTimeZoneColumnIndex = 7;

  private:
    void refresh();

    QSqlDatabase db_;
    Book& book_;
    int& user_id_;
    TransactionFilter filter_;
    Transaction sum_transaction_;
};

#endif // TRANSACTIONS_MODEL_H
