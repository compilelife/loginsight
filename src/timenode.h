#ifndef TIMENODE_H
#define TIMENODE_H

#include <QGraphicsItem>

#define TIME_NODE_HEIGHT 80
#define TIME_NODE_DOT_R 4

//TODO:就地编辑而非弹窗编辑备注
//FIXME:双击的时候不要滚动到高亮行
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
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void requestDel(TimeNode* node);
    void selected(TimeNode* node);
private slots:
    void handleDelActionTriggered();
    void setColor(QColor color);
    void pickColor();

private:
    double mWidth;
    int mLineNum;
    QColor mColor{Qt::blue};
    TimeNodeBody* mBody;
};

#endif // TIMENODE_H
