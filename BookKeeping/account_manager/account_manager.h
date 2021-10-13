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
  virtual void dragEnterEvent(QDragEnterEvent *event) override;
  virtual void dropEvent(QDropEvent *event) override;

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
  void onTreeWidgetItemChanged(QTreeWidgetItem *current);

  void on_pushButton_Add_clicked();
  void on_pushButton_Rename_clicked();
  void on_pushButton_Delete_clicked();

signals:
  void accountNameChanged();
  void categoryChanged();

private:
  Ui::AccountManager* ui_;
  TreeWidget* tree_widget_;
  Book& book_;

  QStringList names_;



  AccountsModel account_model_;
};

#endif // ACCOUNT_MANAGER_H
