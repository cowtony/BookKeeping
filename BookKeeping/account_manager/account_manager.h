#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QPushButton>

#include "book.h"
#include "accounts_model.h"

namespace Ui {
class AccountManager;
}

class TreeWidget : public QTreeWidget {
  Q_OBJECT
public:
  explicit TreeWidget(Book& book, QWidget *parent = nullptr);

protected:
  virtual void startDrag(Qt::DropActions actions) override;
  virtual void dragEnterEvent(QDragEnterEvent *event) override;
  virtual void dropEvent(QDropEvent *event) override;

public slots:
  void onItemChanged(QTreeWidgetItem *item, int column);

private:
  QStringList drag_from_;
  Book& book_;
};

class AccountManager : public QMainWindow {
  Q_OBJECT
public:
  explicit AccountManager(Book& book, QWidget *parent = nullptr);
  ~AccountManager();

private slots:
  void onCurrentItemChanged(QTreeWidgetItem *current);

  void on_pushButton_Add_clicked();
  void on_pushButton_Rename_clicked();
  void on_pushButton_Delete_clicked();

signals:
  void accountNameChanged();
  void categoryChanged();

private:
  QTreeWidgetItem* AddAccountType(Account::Type account_type);
  QTreeWidgetItem* AddAccountGroup(QTreeWidgetItem* category, const QString& group_name);
  QTreeWidgetItem* AddAccount(QTreeWidgetItem* group, const QString& account_name, const QString& comment = "");

  Ui::AccountManager* ui_;
  TreeWidget* tree_widget_;
  QTreeView* tree_view_;
  Book& book_;

  QStringList names_;

  AccountsModel account_model_;
};

#endif // ACCOUNT_MANAGER_H
