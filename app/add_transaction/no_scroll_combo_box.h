#ifndef NO_SCROLL_COMBO_BOX_H
#define NO_SCROLL_COMBO_BOX_H

#include <QComboBox>
#include <QWheelEvent>

class NoScrollComboBox : public QComboBox {
    Q_OBJECT

public:
    NoScrollComboBox(QWidget *parent = nullptr) : QComboBox(parent) {}

protected:
    void wheelEvent(QWheelEvent *event) override {
        // Do not call the base class implementation
        // QComboBox::wheelEvent(event);
        // Instead, just ignore the event
        event->ignore();
    }
};

#endif // NO_SCROLL_COMBO_BOX_H
