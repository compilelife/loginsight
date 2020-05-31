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

    //红色、蓝色、绿色、橘色、黄色、更多（从调色板选择颜色）
    auto colorMenu = menu->addMenu("设置颜色");
    QString names[] = {"红色", "绿色", "蓝色", "黄色", "黑色"};
    QColor colors[] = {Qt::red, Qt::green, Qt::blue, Qt::darkYellow, Qt::black};
    auto count = sizeof (names) / sizeof (QString);
    for (unsigned i = 0; i < count; i++) {
        QPixmap redPixmap(10,10);
        auto color = colors[i];
        redPixmap.fill(colors[i]);
        action = colorMenu->addAction(QIcon(redPixmap), names[i]);
        connect(action, &QAction::triggered, [this, color]{
           this->setColor(color);
        });
    }
    action =  colorMenu->addAction("更多...");
    connect(action, &QAction::triggered, this, &TimeNode::pickColor);

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

void TimeNode::pickColor()
{
    auto color = QColorDialog::getColor();
    if (color.isValid()) {
        setColor(color);
    }
}
