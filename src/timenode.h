#ifndef TIMENODE_H
#define TIMENODE_H

#include <QGraphicsItem>

#define TIME_NODE_HEIGHT 110
#define TIME_NODE_DOT_R 4

class TimeNodeBody;
class TimeNode: public QGraphicsObject
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    TimeNode(int lineNum, const QString& locateText, const QString& detailText);

public:
    QColor color() {return mColor;}
    int data() {return mLineNum;}
    int lineNumber() {return mLineNum;}
protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void requestDel(TimeNode* node);
    void selected(TimeNode* node);
private slots:
    void handleDelActionTriggered();
    void setColor(QColor color);

private:
    double mWidth;
    int mLineNum;
    QColor mColor{Qt::blue};
    TimeNodeBody* mBody;
};

#endif // TIMENODE_H
