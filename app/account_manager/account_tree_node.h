#ifndef ACCOUNTTREENODE_H
#define ACCOUNTTREENODE_H

// Note: Used for accounts_model.
#include <QList>

#include "book/account.h"

class AccountTreeNode {
public:
    AccountTreeNode(const QString& name = QString()) : depth_(0), parent_(nullptr), name_(name) {}
    ~AccountTreeNode();

    // Getters.
    AccountTreeNode* childAt(int index) const;
    AccountTreeNode* childAt(const QString& name) const;
    AccountTreeNode* parent() const { return parent_; }
    int depth() const { return depth_; }
    int childCount() const { return children_.size(); }
    int index() const;
    QString name() const { return name_; }
    QSharedPointer<Account> account() const { return account_; }
    Account::Type accountType() const;
    QString accountGroup() const;

    // Setters.
    bool insertChild(AccountTreeNode* child_node, int index = -1 /* default to append */);
    void removeChildren(int index, int count);
    bool setName(const QString& name);
    void setAccount(QSharedPointer<Account> account) { account_ = account; }
    void clear();

private:
    int childIndex(const QString& name) const;

    int depth_;  // 0: root; 1: Account type; 2: Account group; 3: Account name
    AccountTreeNode* parent_;
    QList<AccountTreeNode*> children_;

    // Tree Node Data:
    QString name_; // Can be account_type_name, category_name, account_name

    // Only valid for account level (depth_ = 3):
    QSharedPointer<Account> account_;  // This will be storing category_id if `depth_ == 2`
};

#endif // ACCOUNTTREENODE_H
