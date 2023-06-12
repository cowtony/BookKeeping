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

    QSqlTableModel model_;

private slots:
    void onPushButtonAddClicked();
    void onPushButtonDeleteClicked();

private:
    bool nameExistsInOtherTable(const QString& name);

    Ui::HouseholdManager* ui;

    int& user_id_;
};

#endif // HOUSEHOLD_MANAGER_H
