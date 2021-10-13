#include "account_tree_node.h"

AccountTreeNode::~AccountTreeNode() {
  for (AccountTreeNode* child : children_) {
    delete child;
  }
}

AccountTreeNode* AccountTreeNode::childAt(int index) const {
  if (index >= 0 and index < children_.size()) {
    return children_.at(index);
  } else {
    return nullptr;
  }
}

AccountTreeNode* AccountTreeNode::childAt(const QString& name) const {
  int index = childIndex(name);
  if (index != -1) {
    return children_.at(index);
  } else {
    return nullptr;
  }
}

int AccountTreeNode::index() const {
  if (!parent_) {
    return 0;
  }
  int index = parent_->childIndex(name_);
  Q_ASSERT(index != -1);
  return index;
}

Account AccountTreeNode::account() const {
  if (depth_ != 3) {
    return Account("Invalid", "Invalid", "Invalid");
  }
  QString category = parent_->name_;
  QString type = parent_->parent_->name_;
  return Account(type, category, name_);
}

bool AccountTreeNode::insertChild(AccountTreeNode* child_node, int index) {
  if (!child_node or depth_ >= 3) {
    return false;
  }
  if (childIndex(child_node->name_) != -1) {
    return false;  // Contains the child with same name.
  }
  if (index == -1) {
    index = children_.size();  // Append.
  }
  if (index < 0 or index > children_.size()) {
    return false;
  }
  child_node->parent_ = this;
  child_node->depth_ = depth_ + 1;
  children_.insert(index, child_node);
  return true;
}

void AccountTreeNode::removeChildren(int index, int count) {
  // TODO: boundary check.
  children_.remove(index, count);
}

bool AccountTreeNode::setName(const QString& name) {
  if (!parent_ or parent_->childIndex(name) != -1) {
    return false;  // Found duplicate name.
  }
  name_ = name;
  return true;
}

int AccountTreeNode::childIndex(const QString& name) const {
  for (int i = 0; i < children_.size(); ++i) {
    if (children_.at(i)->name_ == name) {
      return i;
    }
  }
  return -1;
}
