#ifndef ACCOUNTTREENODE_H
#define ACCOUNTTREENODE_H

// Note: Used for accounts_model.
#include "book/account.h"
#include <QList>

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
    QString comment() const { return comment_; }
    bool isInvestment() const { return is_investment_; }
    std::shared_ptr<Account> account() const;
    QString accountType() const;
    QString accountGroup() const;

    // Setters.
    bool insertChild(AccountTreeNode* child_node, int index = -1 /* default to append */);
    void removeChildren(int index, int count);
    bool setName(const QString& name);
    void setComment(const QString& comment) { comment_ = comment; }
    void setIsInvestment(bool is_investment);
    void clear();

  private:
    int childIndex(const QString& name) const;

    int depth_;  // 0: root; 1: Account type; 2: Account group; 3: Account name
    AccountTreeNode* parent_;
    QList<AccountTreeNode*> children_;
    // Tree Node Data:
    QString name_;
    QString comment_;
    bool is_investment_;
};

#endif // ACCOUNTTREENODE_H
