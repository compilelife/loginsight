#ifndef TIMENODE_H
#define TIMENODE_H

#include <QGraphicsItem>
#include <QJsonObject>

#define TIME_NODE_HEIGHT 110
#define TIME_NODE_DOT_R 4

class TimeNodeBody;
class TimeNode: public QGraphicsObject
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    TimeNode(int lineNum, const QString& locateText, const QString& detailText, const QString& memo = "备注: ");
    TimeNode(const QJsonValue& jo);

public:
    QJsonValue saveToJson();

public:
    QColor color() {return mColor;}
    int data() {return mLineNum;}
    int lineNumber() {return mLineNum;}
protected:
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

signals:
    void requestDel(TimeNode* node);
    void selected(TimeNode* node);
private slots:
    void handleDelActionTriggered();
    void setColor(QColor color);

private:
    QGraphicsRectItem* mHlRect;
    double mWidth;
    int mLineNum;
    QColor mColor{Qt::blue};
    TimeNodeBody* mBody;
    QJsonObject mProjectData;
};

#endif // TIMENODE_H
