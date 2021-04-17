#include "timenodehead.h"
#include <QPainter>
#include "timeline.h"
#include "timenode.h"

TimeNodeHead::TimeNodeHead(TimeNode *node, const QString &text)
    :mText(text), mNode(node){
}

QRectF TimeNodeHead::boundingRect() const {
    return QRectF(0, 0, LINE_X + TIME_NODE_DOT_R, TIME_NODE_HEIGHT);
}

void TimeNodeHead::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QRectF textRect(0, 0, LINE_X, TIME_NODE_HEIGHT);
    painter->setPen(mNode->color());
    painter->drawText(textRect, mText, QTextOption(Qt::AlignCenter));

    painter->setPen(Qt::NoPen);
    painter->setBrush(mNode->color());
    painter->drawEllipse(QPointF(LINE_X, TIME_NODE_HEIGHT/2), TIME_NODE_DOT_R, TIME_NODE_DOT_R);
}
