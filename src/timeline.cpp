#include "timeline.h"
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsDropShadowEffect>
#include "timenode.h"
#include <QImage>

TimeLine::TimeLine(QWidget* parent)
    :QGraphicsView(parent)
{
    setFrameStyle(QFrame::NoFrame);
    QGraphicsScene* scene = new QGraphicsScene;
    mWidth = 350;
    mHeight = 400;
    scene->setSceneRect(0,0,mWidth,mHeight);
    setBackgroundBrush(QBrush(QColor(250,250,250)));
    setScene(scene);

    mLineX = LINE_X;
    mLineY = 4;
    const double d = mLineY;
    auto lineHead = scene->addEllipse({(double)mLineX-d/2, 0, d, d}, Qt::NoPen, QColor(180,180,180));

    mLine = new QGraphicsLineItem();
    mLine->setLine(mLineX, mLineY, mLineX, mHeight);
    mLine->setPen(QPen(QColor(180,180,180)));
    scene->addItem(mLine);

    mNodeTop = 20;

    lineHead->ensureVisible();
}

void TimeLine::addNode(int lineNum, const QString &text) {
    //找到插入位置
    int pos = mNodes.size();
    for (int i = 0; i < mNodes.size(); i++) {
        auto order = mNodes.at(i)->order();
        if (lineNum < order) {
            pos = i;
            break;
        } else if (lineNum == order) {
            return;
        }
    }

    //大于该位置的节点后移
    for (int i = pos; i < mNodes.size(); i++) {
        mNodes.at(i)->setY(calNodeY(i+1));
    }

    //插入新的节点
    auto node = new TimeNode(lineNum, QString("%1").arg(lineNum), text);
    mNodes.insert(pos, node);

    auto minHeight = calNodeY(mNodes.size());
    if (mHeight < minHeight) {
        mHeight = minHeight;
        scene()->setSceneRect(0,0, mWidth, mHeight);
        mLine->setLine(mLineX, mLineY, mLineX, mHeight);
    }

    node->setX(0);
    node->setY(calNodeY(pos));

    scene()->addItem(node);
    node->ensureVisible(0,0,10,10);

    connect(node, SIGNAL(requestDel(TimeNode*)), this, SLOT(deleteNode(TimeNode*)));
    connect(node, SIGNAL(selected(TimeNode*)), this, SIGNAL(nodeSelected(TimeNode*)));
}

//TODO:截图加阴影，加水印(created by xxx)，加文件名
void TimeLine::exportToImage(const QString& path)
{
    auto rect = sceneRect();
    //TODO：高分屏才需要放大2倍吧？
    QImage img((int)(rect.width()*2), (int)(rect.height()*2), QImage::Format_RGB32);

    QPainter painter(&img);
    painter.fillRect(img.rect(), QColor(250,250,250));
    scene()->render(&painter);

    img.save(path);
}

void TimeLine::deleteNode(TimeNode *node)
{
    scene()->removeItem(node);
    mNodes.removeOne(node);

    for (int i = 0; i < mNodes.size(); i++) {
        mNodes.at(i)->setY(calNodeY(i));
    }
}

int TimeLine::calNodeY(int index)
{
    return index * mNodeStep + mNodeTop;
}
