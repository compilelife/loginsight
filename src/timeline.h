#ifndef TIMELINEEX_H
#define TIMELINEEX_H

#include <QGraphicsView>
#include "timenode.h"
#include <QList>
#include <functional>
#include <QTimeLine>
#include <QJsonObject>
#include "ilog.h"

#define LINE_X 70
#define MARGIN_LEFT 10

class TimeLine: public QGraphicsView
{
    Q_OBJECT
public:
    TimeLine(QWidget* parent = nullptr);
    void highlightNode(int lineNum);

public:
    QJsonValue saveToJson();
    void loadFromJson(const QJsonValue &jo);

public slots:
    void addNode(int lineNum, const QString& text);
    void exportToImage(const QString& path);
    void exportToClipboard();
    void clear();

signals:
    void nodeSelected(TimeNode* node);

private slots:
    void deleteNode(TimeNode* node);

private:
    void addNode(TimeNode* node);
    int calNodeY(int index);
    void fitLine();
    void withExportedImage(std::function<void(QImage&)> handler);

    void showSupport();
    void hideSupport();

    void highlightItem(QGraphicsObject *item);

private:
    QGraphicsLineItem* mLine;
    QGraphicsEllipseItem* mLineHead;
    int mWidth;
    int mHeight;
    int mNodeTop;
    int mNodeStep{TIME_NODE_HEIGHT+20};
    int mLineX;
    int mLineY;
    QList<TimeNode*> mNodes;
};

#endif // TIMELINEEX_H
