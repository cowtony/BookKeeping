#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QPushButton>

#include "book/book.h"
#include "accounts_model.h"

namespace Ui {
class AccountManager;
}

class AccountManager : public QMainWindow {
    Q_OBJECT
  public:
    explicit AccountManager(QWidget *parent);
    ~AccountManager();

  private slots:
    void onCurrentItemChanged(const QModelIndex& current, const QModelIndex& previous);
    void onReceiveErrorMessage(const QString& message);

    void on_pushButton_Add_clicked();
    void on_pushButton_Delete_clicked();

  signals:

  private:
    Ui::AccountManager* ui;
    Book& book_;
    AccountsModel account_model_;
};

#endif // ACCOUNT_MANAGER_H
