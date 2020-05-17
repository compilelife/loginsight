#ifndef TIMELINEEX_H
#define TIMELINEEX_H

#include <QGraphicsView>
#include "timenode.h"
#include <QList>
#include <functional>
#include <QTimeLine>

#define LINE_X 70

//TODO:设置自定义着色器改变Node的颜色（比如用来区分来自不同进程的消息）
//TODO:设置自定义的locate提取，比如提取时间，作为locate信息，而非用行号
//TODO:设置自定义的文本提取器，比如删除一些前缀字符，而显示主要内容，而非全部显示
class TimeLine: public QGraphicsView
{
    Q_OBJECT
public:
    TimeLine(QWidget* parent);
    void highlightNode(int lineNum);

public slots:
    void addNode(int lineNum, const QString& text);
    void exportToImage(const QString& path);
    void exportToClipboard();

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
