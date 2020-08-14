#ifndef TIMENODEBODY_H
#define TIMENODEBODY_H

#include <QGraphicsItem>

#define TIME_NODE_BODY_WIDTH 250

class TimeNode;
class TimeNodeBody: public QGraphicsObject
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    TimeNode* mNode;
    QGraphicsRectItem* mColorRect;
    QGraphicsTextItem* mMemo;
public:
    TimeNodeBody(TimeNode* node, const QString& text);
protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
};

#endif // TIMENODEBODY_H
