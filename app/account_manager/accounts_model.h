#ifndef ACCOUNTS_MODEL_H
#define ACCOUNTS_MODEL_H

#include <QAbstractItemModel>

#include "account_tree_node.h"
#include "book/book.h"

const QFont kTypeFont     = QFont("Georgia",         12, 1, true);
const QFont kCategoryFont = QFont("Times New Roman", 12, 1, false);
const QFont kAccountFont  = QFont();

class AccountsModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit AccountsModel(QObject *parent);
    ~AccountsModel();

    void setupCategoriesAndAccounts(const QList<QSharedPointer<Account>>& categories, const QList<QSharedPointer<Account>>& accounts); // Categories are borrowing Account class.
    static AccountTreeNode* getItem(const QModelIndex& index);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

//    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
//    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

//    bool canFetchMore(const QModelIndex &parent) const override;
//    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    // For set up drag & drop.
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    QModelIndex appendRow(const QModelIndex& parent, const QString& name, QSharedPointer<Account> account);  // Third argument is meta_data.
//    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeItem(const QModelIndex& index);
//    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

signals:
    void errorMessage(const QString&);

private:
    AccountTreeNode* root_;
    Book& book_;
    int& user_id_;
};

#endif // ACCOUNTS_MODEL_H
