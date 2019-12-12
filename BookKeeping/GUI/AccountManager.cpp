#include "AccountManager.h"
#include "ui_AccountManager.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDropEvent>
#include "Book.h"

TreeWidget::TreeWidget(QWidget *parent): QTreeWidget(parent) {
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
}

void TreeWidget::dragEnterEvent(QDragEnterEvent *event) {
    dragFrom.clear();
    QTreeWidgetItem *item = currentItem();
    while (item != nullptr) {
        dragFrom.push_front(item->text(0));
        item = item->parent();
    }
    QTreeWidget::dragEnterEvent(event);
}

void TreeWidget::dropEvent(QDropEvent *event) {
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid()) {  // just in case
        event->setDropAction(Qt::IgnoreAction);  // This may not necessary
        return;
    }

    QTreeWidgetItem* item = itemFromIndex(index);
    QStringList dragTo;

    while (item != nullptr) {
        dragTo.push_front(item->text(0));
        item = item->parent();
    }

    if (dragFrom.size() != 3 || dragTo.size() != 3)
        return;
    else if (dragFrom.at(0) != dragTo.at(0))
        return;
    else if (dragFrom.at(1) != dragTo.at(1)) {
        if (g_book.accountExist(Account(dragTo.at(0), dragTo.at(1), dragFrom.at(2)))) {
            QMessageBox messageBox;
            messageBox.setText("Do you want to merge with existing account?");
            messageBox.setInformativeText("Old Category: " + dragFrom.at(1) + ", New Category: " + dragTo.at(1));
            messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            messageBox.setDefaultButton(QMessageBox::Ok);
            switch (messageBox.exec()) {
            case QMessageBox::Ok:
                QApplication::setOverrideCursor(Qt::WaitCursor);
                if (g_book.moveAccount(Account(dragFrom.at(0), dragFrom.at(1), dragFrom.at(2)),
                                       Account(dragTo.at(0),   dragTo.at(1), dragFrom.at(2)))) {
                    delete currentItem();
                }
                QApplication::restoreOverrideCursor();
                return;
            case QMessageBox::Discard:
                return;
            }
        } else {
            QMessageBox messageBox;
            messageBox.setText("You are trying to move item '" + dragFrom.at(2)
                               + "' from '" + dragFrom.at(1)
                               + "' to '" + dragTo.at(1) + "'.");
            messageBox.setInformativeText("Do you want to continue?");
            messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
            messageBox.setDefaultButton(QMessageBox::Ok);
            switch (messageBox.exec()) {
            case QMessageBox::Ok:
                QApplication::setOverrideCursor(Qt::WaitCursor);
                g_book.moveAccount(Account(dragFrom.at(0), dragFrom.at(1), dragFrom.at(2)),
                                   Account(dragTo.at(0),   dragTo.at(1), dragFrom.at(2)));
                QApplication::restoreOverrideCursor();
                break;
            case QMessageBox::Discard:
                return;
            }
        }

        qDebug() << dragFrom << dragTo;
    }
    QTreeWidget::dropEvent(event);
}

AccountManager::AccountManager(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AccountManager)
{
    ui->setupUi(this);

    for (const Account::TableType &tableType : {Account::Asset, Account::Liability, Account::Expense, Account::Revenue})
    {
        QTreeWidgetItem* accountTypeItem = new QTreeWidgetItem(ui->treeWidget);
        accountTypeItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable) & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
        accountTypeItem->setText(0, Account::TableName.value(tableType));
        accountTypeItem->setFont(0, m_tableFont);

        for (const QString &category : g_book.getCategories(tableType))
        {
            QTreeWidgetItem* accountCategoryItem = new QTreeWidgetItem(accountTypeItem);
            accountCategoryItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled) & ~Qt::ItemIsDragEnabled);
            accountCategoryItem->setText(0, category);
            accountCategoryItem->setFont(0, m_categoryFont);

            for (const QString &name : g_book.getAccountNames(tableType, category))
            {
                QTreeWidgetItem* accountItem = new QTreeWidgetItem(accountCategoryItem);
                accountItem->setFlags((Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled) & ~Qt::ItemIsDropEnabled);
                accountItem->setText(0, name);
                accountItem->setFont(0, m_accountFont);
            }
        }
    }
}

AccountManager::~AccountManager()
{
//    delete ui;
}

void AccountManager::on_treeWidget_currentItemChanged(QTreeWidgetItem *current)
{
    QTreeWidgetItem* node = current;

    names.clear();
    while (node != nullptr)
    {
        names.push_front(node->text(0));
        node = node->parent();
    }
    switch (names.size())
    {
    case 1:  // Table
        ui->pushButton_Add   ->setEnabled(true);
        ui->pushButton_Rename->setEnabled(false);
        ui->pushButton_Delete->setEnabled(false);
        break;
    case 2:  // Category
        ui->pushButton_Add   ->setEnabled(true);
        ui->pushButton_Rename->setEnabled(true);
        ui->pushButton_Delete->setEnabled(true);
        break;
    case 3:  // Name
        ui->pushButton_Add   ->setEnabled(false);
        ui->pushButton_Rename->setEnabled(true);
        ui->pushButton_Delete->setEnabled(true);
        break;
    }
}

void AccountManager::on_pushButton_Add_clicked()
{
    switch (names.size())
    {
    case 1:  // Table Level
    {
        bool ok;
        QString category = QInputDialog::getText(this, "Add category into table " + names.at(0), "Category:", QLineEdit::Normal, "", &ok);
        if (ok)
        {
            if (g_book.insertCategory(names.at(0), category))
            {
                QTreeWidgetItem* categoryItem = new QTreeWidgetItem(ui->treeWidget->currentItem());
                categoryItem->setText(0, category);
                categoryItem->setFont(0, m_categoryFont);
                for (const QString &accountName : g_book.getAccountNames(Account::TableName.key(names.at(0)), category))
                {
                    QTreeWidgetItem* accountItem = new QTreeWidgetItem(categoryItem);
                    accountItem->setText(0, accountName);
                    accountItem->setFont(0, m_accountFont);
                }
            }
        }
        break;
    }
    case 2:  // Category Level
    {
        bool ok;
        QString accountName = QInputDialog::getText(this, "Add Account into category " + names[1], "Name:", QLineEdit::Normal, "", &ok);
        if (ok && !accountName.isEmpty())
        {
            if (g_book.insertAccount(Account(names.at(0), names.at(1), accountName)))
            {
                QTreeWidgetItem* l_item = new QTreeWidgetItem;
                ui->treeWidget->currentItem()->addChild(l_item);
                l_item->setText(0, accountName);
                l_item->setFont(0, m_accountFont);
            }
            else {
                qDebug() << Q_FUNC_INFO << "Insert failed" << names.at(0) << names.at(1) << accountName;
            }
        }
        else {
            qDebug() << Q_FUNC_INFO << ok << accountName;
        }
        break;
    }
    case 3:  // Account Level
        break;
    }
}

void AccountManager::on_pushButton_Rename_clicked()
{
    switch (names.size())
    {
    case 1:  // Table Level
        break;
    case 2:  // Category Level
    {
        bool ok;
        QString newName = QInputDialog::getText(this, "Rename Category", "Name:", QLineEdit::Normal, names.at(1), &ok);
        if (ok && !newName.isEmpty())
        {
            if (g_book.categoryExist(names.at(0), newName))
            {
                QMessageBox messageBox;
                messageBox.setText("Error: The category already exist.");
                messageBox.exec();
            }
            else
            {
                QApplication::setOverrideCursor(Qt::WaitCursor);
                if (g_book.renameCategory(names.at(0), names.at(1), newName))
                {
                    ui->treeWidget->currentItem()->setText(0, newName);
                    names[1] = newName;
                    emit categoryChanged();
                }
                QApplication::restoreOverrideCursor();
            }
        }
        break;
    }
    case 3:  // Account Level
    {
        bool ok;
        QString newName = QInputDialog::getText(this, "Rename Account", "Name:", QLineEdit::Normal, names.at(2), &ok);
        if (ok && !newName.isEmpty())
        {
            if (g_book.accountExist(Account(names.at(0), names.at(1), newName)))
            {
                QMessageBox messageBox;
                messageBox.setText("Do you want to merge with existing account?");
                messageBox.setInformativeText("OldName: " + names[2] + ", NewName: " + newName);
                messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
                messageBox.setDefaultButton(QMessageBox::Ok);
                switch (messageBox.exec()) {
                case QMessageBox::Ok:
                    QApplication::setOverrideCursor(Qt::WaitCursor);
                    if (g_book.moveAccount(Account(names.at(0), names.at(1), names.at(2)),
                                           Account(names.at(0), names.at(1), newName)))
                    {
                        delete ui->treeWidget->currentItem();
                        on_treeWidget_currentItemChanged(ui->treeWidget->currentItem());
                        emit accountNameChanged();
                    }
                    QApplication::restoreOverrideCursor();
                    return;
                case QMessageBox::Discard:
                    return;
                }
            }
            else if (g_book.moveAccount(Account(names.at(0), names.at(1), names.at(2)),
                                        Account(names.at(0), names.at(1), newName)))
            {
                ui->treeWidget->currentItem()->setText(0, newName);
                names[2] = newName;
                emit accountNameChanged();
            }
        }
        break;
    }
    }
}

void AccountManager::on_pushButton_Delete_clicked()
{
    switch (names.size())
    {
    case 1:  // Table Level
        break;
    case 2:  // Category Level
        if (g_book.removeCategory(names.at(0), names.at(1)))
        {
            delete ui->treeWidget->currentItem();
        }
        else
        {
            QMessageBox messageBox;
            messageBox.setText("Cannot delete!\nThe category still have account in it!");
            messageBox.exec();
        }
        break;
    case 3:  // Account Level
        if (g_book.removeAccount(Account(names.at(0), names.at(1), names.at(2))))
        {
            delete ui->treeWidget->currentItem();
        }
        else {
            QMessageBox messageBox;
            messageBox.setText("Cannot delete!\nThe account still have transactions link to it!");
            messageBox.exec();
        }
        break;
    }
}


