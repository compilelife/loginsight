#include "timenodebody.h"
#include <QTextOption>
#include <QPainter>
#include "timenode.h"
#include <QDebug>
#include <QInputDialog>
#include <QStyleOptionGraphicsItem>

TimeNodeBody::TimeNodeBody(const QString &text) {
    mText = text;
}

QRectF TimeNodeBody::boundingRect() const {
    return QRectF(0, 0, TIME_NODE_BODY_WIDTH, TIME_NODE_HEIGHT);
}

void TimeNodeBody::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    auto r = boundingRect();
    painter->fillRect(r, Qt::white);

    QRectF colorRect(r.x(), r.y(), 2, r.height());
    painter->fillRect(colorRect, Qt::red);

    const int margin = 4;
    QRectF textRect(r.x() + margin + colorRect.width(),
                    r.y() + margin,
                    r.width() - 2 * margin - colorRect.width(),
                    r.height() - 2* margin);

    auto fontHeight = option->fontMetrics.height();
    auto memoHeight = fontHeight * 1.2;
    QRectF contentRect(textRect.x(), textRect.y(), textRect.width(), textRect.height() - memoHeight);
    QRectF memoRect(textRect.x(), contentRect.bottom(), textRect.width(), memoHeight);

    QTextOption op(Qt::AlignLeft);
    op.setWrapMode(QTextOption::WrapAnywhere);
    painter->setPen(QColor(100,100,100));
    painter->drawText(contentRect, mText, op);

    op.setWrapMode(QTextOption::NoWrap);
    op.setAlignment(Qt::AlignVCenter);
    painter->setPen(Qt::blue);
    painter->drawText(memoRect, "备注："+mMemo, op);
}

void TimeNodeBody::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    auto memo = QInputDialog::getText(nullptr, "设置备注", "请输入备注:", QLineEdit::Normal, mMemo);
    if (!memo.isNull()) {
        mMemo = memo;
        update();
    }
    QGraphicsItem::mouseDoubleClickEvent(event);
}
