#include "account_tree_node.h"

AccountTreeNode::~AccountTreeNode() {
  clear();
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

Account::Type AccountTreeNode::accountType() const {
    switch (depth_) {
        case 1: return Account::kAccountTypeName.key(name_);
        case 2: return Account::kAccountTypeName.key(parent_->name_);
        case 3: return Account::kAccountTypeName.key(parent_->parent_->name_);
        default: return Account::INVALID;
    }
}

QString AccountTreeNode::accountGroup() const {
    switch (depth_) {
        case 2: return name_;
        case 3: return parent_->name_;
        default: return "";
    }
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
  for (int i = 0; i < count; ++i) {
    if (index < 0 or index >= children_.size()) {
      return;  // Out of boundary.
    }
    AccountTreeNode* item = children_.at(index);
    children_.remove(index);
    delete item;
  }
}

bool AccountTreeNode::setName(const QString& name) {
  if (!parent_ or parent_->childIndex(name) != -1) {
    return false;  // Found duplicate name.
  }
  this->name_ = name;
  return true;
}

void AccountTreeNode::clear() {
  for (AccountTreeNode* child : children_) {
    delete child;
  }
  children_.clear();
}

int AccountTreeNode::childIndex(const QString& name) const {
  for (int i = 0; i < children_.size(); ++i) {
    if (children_.at(i)->name() == name) {
      return i;
    }
  }
  return -1;
}
