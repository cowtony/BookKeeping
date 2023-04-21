
#ifndef JSONDELEGATE_H
#define JSONDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>


class JsonDelegate : public QStyledItemDelegate {
  public:
    JsonDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
#endif // JSONDELEGATE_H
