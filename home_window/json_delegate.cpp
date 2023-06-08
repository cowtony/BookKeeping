#include "json_delegate.h"

void JsonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);
//    QStyledItemDelegate::paint(painter, option, QModelIndex()); // Clear the default rendering

//    QVariant data = index.data();
//    QJsonObject jsonObj;
//    if (data.typeId() == QMetaType::QString) {
//        jsonObj = QJsonDocument::fromJson(data.toString().toUtf8()).object();
//    } else if (data.typeId() == QMetaType::QJsonObject) {
//        jsonObj = data.toJsonObject();
//    }

//    QStringList lines;
//    for (const QString& account_name : jsonObj.keys()) {
//        lines.append(account_name + ":" + jsonObj.value(account_name).toString());
//    }
//    painter->drawText(option.rect, option.displayAlignment, index.data().toString().replace(R"(",)", "\",\n"));
}
