#include "timenode.h"
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include "timeline.h"
#include "timenodebody.h"
#include "timenodehead.h"

TimeNode::TimeNode(int lineNum, const QString &locateText, const QString &detailText)
    :mLineNum(lineNum) {
//    setFlag(GraphicsItemFlag::ItemIsSelectable);

    auto head = new TimeNodeHead(locateText);
    head->setX(0);
    head->setY(0);
    head->setParentItem(this);

    mWidth = LINE_X + TIME_NODE_DOT_R;

    auto body = new TimeNodeBody(detailText);
    body->setX(mWidth + 8);
    body->setY(0);
    body->setParentItem(this);

    mWidth += (8 + TIME_NODE_BODY_WIDTH);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(2,2);
    shadow->setBlurRadius(10);
    body->setGraphicsEffect(shadow);

    mWidth += 2;
}

QRectF TimeNode::boundingRect() const
{
    return QRectF(0,0, mWidth, TIME_NODE_HEIGHT);
}

void TimeNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

}

void TimeNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    auto menu = new QMenu();

    auto action = menu->addAction("删除");
    connect(action, SIGNAL(triggered()), this, SLOT(handleDelActionTriggered()));

    menu->exec(event->screenPos());
    delete menu;
}

void TimeNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit selected(this);
    QGraphicsItem::mousePressEvent(event);
}

void TimeNode::handleDelActionTriggered()
{
    emit requestDel(this);
}
