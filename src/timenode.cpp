#include "timenode.h"
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsBlurEffect>
#include <QDebug>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include "timeline.h"
#include "timenodebody.h"
#include "timenodehead.h"
#include <QColorDialog>
#include "choosecolormenu.h"

TimeNode::TimeNode(int lineNum, const QString &locateText, const QString &detailText, const QString& memo)
    :mLineNum(lineNum) {
    setFlag(GraphicsItemFlag::ItemIsSelectable);

    auto head = new TimeNodeHead(this, locateText);
    head->setX(0);
    head->setY(0);
    head->setParentItem(this);

    mWidth = LINE_X + TIME_NODE_DOT_R;

    mBody = new TimeNodeBody(this, detailText, memo);
    mBody->setX(mWidth + 8);
    mBody->setY(0);
    mBody->setParentItem(this);

    mWidth += (8 + TIME_NODE_BODY_WIDTH);

    mWidth += 2;

    mHlRect = new QGraphicsRectItem(-6,-6,mWidth+12, TIME_NODE_HEIGHT+12, this);
    QPen pen(Qt::red);
    pen.setWidth(2);
    mHlRect->setPen(pen);
    mHlRect->setVisible(false);
    auto blur = new QGraphicsBlurEffect(this);
    blur->setBlurRadius(4);
    mHlRect->setGraphicsEffect(blur);

    mProjectData["lineNum"] = lineNum;
    mProjectData["detail"] = detailText;
}

TimeNode::TimeNode(const QJsonValue &jo)
    :TimeNode(jo["lineNum"].toInt(),
      QString("%1").arg(jo["lineNum"].toInt()),
      jo["detail"].toString(),
      jo["memo"].toString())
{
    setColor(jo["color"].toString());
}

QJsonValue TimeNode::saveToJson()
{
    mProjectData["memo"] = mBody->getMemo();
    mProjectData["color"] = mColor.name();
    return mProjectData;
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
    setSelected(true);
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

void TimeNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit selected(this);
    QGraphicsItem::mousePressEvent(event);
}

QVariant TimeNode::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        auto selected = value.toBool();
        mHlRect->setVisible(selected);
    }
    return value;
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

