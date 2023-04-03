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

class AccountManager : public QMainWindow {
    Q_OBJECT
  public:
    explicit AccountManager(Book& book, QWidget *parent = nullptr);
    ~AccountManager() = default;

  private slots:
    void onCurrentItemChanged(const QModelIndex& current, const QModelIndex& previous);
    void onReceiveErrorMessage(const QString& message);

    void on_pushButton_Add_clicked();
    void on_pushButton_Delete_clicked();

  signals:

  private:
    std::shared_ptr<Ui::AccountManager> ui_;
    Book& book_;
//    TreeWidget* tree_widget_;
    std::unique_ptr<QTreeView> tree_view_;
    AccountsModel account_model_;
};

/* This was deprecated code, replaced with ModelView.
 *
class TreeWidget : public QTreeWidget {
  Q_OBJECT
public:
  explicit TreeWidget(Book& book, QWidget *parent = nullptr);

protected:
  virtual void startDrag(Qt::DropActions actions) override;
  virtual void dragEnterEvent(QDragEnterEvent *event) override;
  virtual void dropEvent(QDropEvent *event) override;

private:
  QStringList drag_from_;
  Book& book_;
};
*/

#endif // ACCOUNT_MANAGER_H
