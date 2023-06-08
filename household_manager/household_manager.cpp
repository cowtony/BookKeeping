#include "household_manager.h"
#include "ui_household_manager.h"

#include "home_window/home_window.h"
#include <QMessageBox>
#include <QInputDialog>

HouseholdManager::HouseholdManager(QWidget *parent) :
  QMainWindow(parent),
  model_(QSqlTableModel(this, static_cast<HomeWindow*>(parent)->book.db)),
  ui(new Ui::HouseholdManager),
  user_id_(static_cast<HomeWindow*>(parent)->user_id_) {

    ui->setupUi(this);

    model_.setTable("book_household");
    model_.setFilter(QString("user_id = %1").arg(user_id_));
    model_.setSort(3, Qt::AscendingOrder);  // ORDER BY `rank`
    model_.setEditStrategy(QSqlTableModel::OnFieldChange);  // So that the edit to the cell will apply to DB immediatly.

    // Select the data from the table
    model_.select();

    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::DoubleClicked);

    connect(ui->pushButtonAdd,    &QPushButton::clicked, this, &HouseholdManager::onPushButtonAddClicked);
    connect(ui->pushButtonDelete, &QPushButton::clicked, this, &HouseholdManager::onPushButtonDeleteClicked);

    ui->tableView->setModel(&model_);
    ui->tableView->hideColumn(0);  // household_id
    ui->tableView->hideColumn(1);  // user_id
}

HouseholdManager::~HouseholdManager() {
    delete ui;
}

void HouseholdManager::onPushButtonAddClicked() {
    bool ok;
    QString name = QInputDialog::getText(this, "Add Row", "Enter Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) {
        return;
    }
    // Insert a new row with the entered name
    int new_row = model_.rowCount();
    if (!model_.insertRow(new_row)) {
        // TODO: Error handling
    }
    if (!model_.setData(model_.index(new_row, 1), user_id_)) {
        // TODO: Error handling
    }
    if (!model_.setData(model_.index(new_row, 2), name)) {
        // TODO: Error handling
    }
    if (!model_.setData(model_.index(new_row, 3), /*rank=*/99)) {
        // TODO: Error handling
    }

    // Submit the changes to the database
    if (model_.submitAll()) {
        qDebug() << "New row added successfully!";
    } else {
        qDebug() << "Failed to add new row:" << model_.lastError().text();
        // Rollback the transaction
        model_.database().rollback();
        // Remove the inserted row
        model_.removeRow(new_row);
        // Show an error message
        QMessageBox::critical(this, "Error", "Failed to add new row: " + model_.lastError().text());
    }
}

bool HouseholdManager::nameExistsInOtherTable(const QString& name) {
    // Check if the name exists in another table
    QSqlQuery query(model_.database());
    query.prepare("SELECT COUNT(*) FROM other_table WHERE name = ?");
    query.addBindValue(name);
    if (query.exec() && query.next()) {
        int count = query.value(0).toInt();
        if (count > 0) {
            return true;
        }
    } else {
        qDebug() << "Failed to check if name exists in other table:" << query.lastError().text();
    }
    return false;
}

void HouseholdManager::onPushButtonDeleteClicked() {
    // Get the selected row in the table
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, "Information", "No row selected to delete.");
        return;
    }

    // Get the index of the selected row
    QModelIndex selectedIndex = selectedIndexes.first();
    int row = selectedIndex.row();

    // Get the name of the row to be deleted
    QString name = model_.index(row, 1).data().toString();

    // Check if the name exists in another table's column
    if (nameExistsInOtherTable(name)) {
        QMessageBox::critical(this, "Error", "Cannot delete row. Name '" + name + "' exists in another table's column.");
        return;
    }

    // Show a confirmation dialog
    QMessageBox::StandardButton result = QMessageBox::question(this, "Confirmation", "Are you sure you want to delete the selected row?", QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes) {
        // Delete the selected row from the model
        model_.removeRow(row);

        // Submit the changes to the database
        if (model_.submitAll()) {
            qDebug() << "Selected row deleted successfully!";
            ui->tableView->reset();
        } else {
            qDebug() << "Failed to delete selected row:" << model_.lastError().text();
        }
    }
}
