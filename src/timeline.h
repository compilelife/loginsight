#ifndef TIMELINEEX_H
#define TIMELINEEX_H

#include <QGraphicsView>
#include "timenode.h"
#include <QList>
#include <functional>
#include <QTimeLine>

#define LINE_X 70

class TimeLine: public QGraphicsView
{
    Q_OBJECT
public:
    TimeLine(QWidget* parent = nullptr);
    void highlightNode(int lineNum);

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
    int calNodeY(int index);
    void fitLine();
    void withExportedImage(std::function<void(QImage&)> handler);

    void showSupport();
    void hideSupport();

    void highlightItem(QGraphicsObject *item);

private:
    QGraphicsLineItem* mLine;
    QGraphicsEllipseItem* mLineHead;
    QGraphicsPixmapItem* mSupportImg;
    QGraphicsTextItem* mSupportText;
    int mWidth;
    int mHeight;
    int mNodeTop;
    int mNodeStep{120};
    int mLineX;
    int mLineY;
    QList<TimeNode*> mNodes;
};

#endif // TIMELINEEX_H
