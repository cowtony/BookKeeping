#ifndef HOUSEHOLD_MANAGER_H
#define HOUSEHOLD_MANAGER_H

#include <QMainWindow>

#include <QSqlTableModel>

namespace Ui {
class HouseholdManager;
}

class HouseholdManager : public QMainWindow {
    Q_OBJECT

public:
    explicit HouseholdManager(QWidget *parent);
    ~HouseholdManager();

private slots:
    void onPushButtonAddClicked();
    void onPushButtonDeleteClicked();
    void onTableViewDoubleCLicked(const QModelIndex& index);

private:
    bool nameExistsInOtherTable(const QString& name);

    Ui::HouseholdManager* ui;
    QSqlTableModel model_;
};

#endif // HOUSEHOLD_MANAGER_H
