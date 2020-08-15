#include "timenodebody.h"
#include <QTextOption>
#include <QPainter>
#include "timenode.h"
#include <QDebug>
#include <QInputDialog>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QGraphicsTextItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsDropShadowEffect>
#include <QTextDocument>

#define MARGIN (2)
#define COLOR_RECT_WIDTH (2)
#define MEMO_HEIGHT (40)
#define CONTENT_WIDTH (TIME_NODE_BODY_WIDTH-MARGIN*2-COLOR_RECT_WIDTH)
#define CONTENT_HEIGHT (TIME_NODE_HEIGHT-MEMO_HEIGHT-2*MARGIN)
//FIXME：没法直接用QGraphicsTextItem，因为高度不可控
class TextItem: public QGraphicsItem
{
private:
    QString mText;
public:
    TextItem(const QString& text, QGraphicsItem* parent):mText(text), QGraphicsItem(parent){}
protected:
    QRectF boundingRect() const override {
        return QRectF(0,0, CONTENT_WIDTH, CONTENT_HEIGHT);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
            QTextOption op(Qt::AlignLeft);
            op.setWrapMode(QTextOption::WrapAnywhere);
            painter->setPen(QColor(100,100,100));
            painter->drawText(boundingRect(), mText, op);
    }
};

TimeNodeBody::TimeNodeBody(TimeNode* node, const QString &text)
    :mNode(node){
    auto rect = new QGraphicsRectItem(0,0,TIME_NODE_BODY_WIDTH, TIME_NODE_HEIGHT, this);
    rect->setBrush(Qt::white);
    rect->setPen(Qt::NoPen);
    auto shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(2,2);
    shadow->setBlurRadius(10);
    rect->setGraphicsEffect(shadow);

    mColorRect = new QGraphicsRectItem(0,0,COLOR_RECT_WIDTH,TIME_NODE_HEIGHT, this);
    mColorRect->setBrush(mNode->color());
    mColorRect->setPen(Qt::NoPen);

    auto content = new TextItem(text, this);
    content->setX(MARGIN+COLOR_RECT_WIDTH);
    content->setY(MARGIN);

    auto line = new QGraphicsLineItem(this);
    line->setLine(MARGIN+COLOR_RECT_WIDTH+10, MARGIN+CONTENT_HEIGHT,
                  MARGIN+COLOR_RECT_WIDTH+CONTENT_WIDTH-20, MARGIN+CONTENT_HEIGHT);
    line->setPen(QPen(Qt::DotLine));

    mMemo = new QGraphicsTextItem("备注: ", this);
    mMemo->setTextInteractionFlags(Qt::TextEditorInteraction);
    mMemo->setTextWidth(CONTENT_WIDTH);
    mMemo->setX(MARGIN+COLOR_RECT_WIDTH);
    mMemo->setDefaultTextColor(mNode->color());
    QTextOption op;
    op.setWrapMode(QTextOption::WrapAnywhere);
    mMemo->document()->setDefaultTextOption(op);
    mMemo->setY(MARGIN+CONTENT_HEIGHT);
}

QRectF TimeNodeBody::boundingRect() const {
    return QRectF(0, 0, TIME_NODE_BODY_WIDTH, TIME_NODE_HEIGHT);
}

void TimeNodeBody::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    mColorRect->setBrush(mNode->color());
    mMemo->setDefaultTextColor(mNode->color());
}
