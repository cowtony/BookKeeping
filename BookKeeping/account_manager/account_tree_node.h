#ifndef ACCOUNTTREENODE_H
#define ACCOUNTTREENODE_H

// Note: Used for accounts_model.

#include "account.h"

class AccountTreeNode {
public:
  AccountTreeNode(const QString& name = QString()) : name_(name), depth_(0), parent_(nullptr) {}
  ~AccountTreeNode();

  AccountTreeNode* childAt(int index) const;
  AccountTreeNode* childAt(const QString& name) const;
  AccountTreeNode* parent() const { return parent_; }
  QString name() const { return name_; }
  int depth() const { return depth_; }
  int childCount() const { return children_.size(); }
  int index() const;
  Account account() const;

  bool insertChild(AccountTreeNode* child_node, int index = -1 /* default to append */);
  void removeChildren(int index, int count);
  bool setName(const QString& name);

private:
  int childIndex(const QString& name) const;

  QString name_;
  int depth_;
  QList<AccountTreeNode*> children_;
  AccountTreeNode* parent_;
};

#endif // ACCOUNTTREENODE_H
