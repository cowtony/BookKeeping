#ifndef BOOKMODEL_H
#define BOOKMODEL_H

#include <QAbstractTableModel>

#include "transaction.h"

const QVector<Account::Type> kAccountTypes = {Account::Expense, Account::Revenue, Account::Asset, Account::Liability};

class BookModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    explicit BookModel(QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
//    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

//    bool canFetchMore(const QModelIndex &parent) const override;
//    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
//    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

//    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
//    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
//    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    void SetTransactions(const QList<Transaction>& transactions);
    const QList<Transaction>& transactions() const { return transactions_; }
    QList<Transaction> getTransactions(const QModelIndexList& index_list) const;
    Transaction getTransaction(const QModelIndex& model_index) const;

    static const int kMaximumTransactions = 200;

  private:
    const int kReservedFilterRow = 2;
    const std::vector<QString> kColumnNames = {"Date", "Description", "Expense", "Revenue", "Asset", "Liability"};

    QList<Transaction> transactions_;
    Transaction sum_transaction_;
};

#endif // BOOKMODEL_H
