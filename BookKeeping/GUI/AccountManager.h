#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QPushButton>

#include "Book.h"

namespace Ui {
class AccountManager;
}

class TreeWidget : public QTreeWidget {
  Q_OBJECT
public:
  explicit TreeWidget(std::shared_ptr<Book> book, QWidget *parent = nullptr);

protected:
  virtual void dragEnterEvent(QDragEnterEvent *event) override;
  virtual void dropEvent(QDropEvent *event) override;

private:
  QStringList drag_from_;
  std::shared_ptr<Book> book_;
};

class AccountManager : public QMainWindow {
  Q_OBJECT
public:
  explicit AccountManager(std::shared_ptr<Book> book, QWidget *parent = nullptr);
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
  std::shared_ptr<Book> book_;

  QStringList names_;

  const QFont kTableFont    = QFont("Georgia",         12, 1, true);
  const QFont kCategoryFont = QFont("Times New Roman", 12, 1, false);
  const QFont kAccountFont  = QFont();
};

#endif // ACCOUNTMANAGER_H
