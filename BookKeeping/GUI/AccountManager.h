#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QPushButton>

namespace Ui {
class AccountManager;
}

class TreeWidget : public QTreeWidget
{
public:
    explicit TreeWidget(QWidget *parent = nullptr);
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
private:
    QStringList dragFrom;
};

class AccountManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit AccountManager(QWidget *parent = nullptr);
    ~AccountManager();

private slots:
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current);

    void on_pushButton_Add_clicked();
    void on_pushButton_Rename_clicked();
    void on_pushButton_Delete_clicked();

signals:
    void accountNameChanged();
    void categoryChanged();

private:
    Ui::AccountManager *ui_;

    QStringList names_;

    const QFont kTableFont    = QFont("Georgia",         12, 1, true);
    const QFont kCategoryFont = QFont("Times New Roman", 12, 1, false);
    const QFont kAccountFont  = QFont();
};

#endif // ACCOUNTMANAGER_H
