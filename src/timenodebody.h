#ifndef TIMENODEBODY_H
#define TIMENODEBODY_H

#include <QGraphicsItem>

#define TIME_NODE_BODY_WIDTH 250

class TimeNodeBody: public QGraphicsItem {
private:
    QString mText;
    QString mMemo;
public:
    TimeNodeBody(const QString& text);
protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // TIMENODEBODY_H
