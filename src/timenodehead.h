#ifndef TIMENODEHEAD_H
#define TIMENODEHEAD_H

#include <QGraphicsItem>

class TimeNodeHead: public QGraphicsItem {
private:
    QString mText;
public:
    TimeNodeHead(const QString& text);
protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
};

#endif // TIMENODEHEAD_H
