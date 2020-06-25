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
#include <QColorDialog>
#include "choosecolormenu.h"

TimeNode::TimeNode(int lineNum, const QString &locateText, const QString &detailText)
    :mLineNum(lineNum) {
//    setFlag(GraphicsItemFlag::ItemIsSelectable);

    auto head = new TimeNodeHead(this, locateText);
    head->setX(0);
    head->setY(0);
    head->setParentItem(this);

    mWidth = LINE_X + TIME_NODE_DOT_R;

    mBody = new TimeNodeBody(this, detailText);
    mBody->setX(mWidth + 8);
    mBody->setY(0);
    mBody->setParentItem(this);

    mWidth += (8 + TIME_NODE_BODY_WIDTH);

    auto shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(2,2);
    shadow->setBlurRadius(10);
    mBody->setGraphicsEffect(shadow);

    mWidth += 2;
}

QRectF TimeNode::boundingRect() const
{
    return QRectF(0,0, mWidth, TIME_NODE_HEIGHT);
}

void TimeNode::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
}

void TimeNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    auto menu = new QMenu();

    auto action = menu->addAction("删除");
    connect(action, SIGNAL(triggered()), this, SLOT(handleDelActionTriggered()));

    menu->addSeparator();

    auto chooseColorMenu = new ChooseColorMenu({
                {"红色", Qt::red},
                {"绿色", Qt::green},
                {"蓝色", Qt::blue},
                {"黄色", Qt::yellow},
                {"黑色", Qt::black},
            });
    connect(chooseColorMenu, &ChooseColorMenu::chooseColor, [this](QColor color){
        setColor(color);
    });
    menu->addMenu(chooseColorMenu);

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

void TimeNode::setColor(QColor color)
{
    mColor = color;
    mBody->update();//FIXME: 不知为何body添加了effect后，没法通过TimeNode::update来强制更新了
    update();
}

