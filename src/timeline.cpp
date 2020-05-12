#include "timeline.h"
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsDropShadowEffect>
#include "timenode.h"
#include <QImage>
#include <QtGlobal>
#include <QApplication>
#include "toast.h"
#include <QClipboard>

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
    mLineY = 8;
    const double d = 4;
    auto lineHead = scene->addEllipse({(double)mLineX-d/2, 4, d, d}, Qt::NoPen, QColor(180,180,180));

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

    fitLine();

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
    withExportedImage([&path](QImage& img){
        img.save(path);
    });
}

void TimeLine::exportToClipboard()
{
    withExportedImage([](QImage& img){
        QApplication::clipboard()->setImage(img);
        Toast::instance().show(Toast::INFO, "时间线已复制到剪贴板");
    });
}

void TimeLine::deleteNode(TimeNode *node)
{
    scene()->removeItem(node);
    mNodes.removeOne(node);

    for (int i = 0; i < mNodes.size(); i++) {
        mNodes.at(i)->setY(calNodeY(i));
    }

    fitLine();
}

int TimeLine::calNodeY(int index)
{
    return index * mNodeStep + mNodeTop;
}

void TimeLine::fitLine()
{
    auto minHeight = calNodeY(mNodes.size())+30;
    if (minHeight < 100)
        minHeight = 100;

    mHeight = minHeight;
    scene()->setSceneRect(0,0, mWidth, mHeight);
    mLine->setLine(mLineX, mLineY, mLineX, mHeight);
}

void TimeLine::withExportedImage(std::function<void (QImage &)> handler)
{
    auto rect = sceneRect();
#ifdef Q_OS_MAC
    int borderMargin = 10;
    //避免图片发虚
    QImage img((int)(rect.width()*2) + 2*borderMargin, (int)(rect.height()*2) + 2*borderMargin, QImage::Format_RGB32);
#else
    int borderMargin = 5;
    QImage img((int)(rect.width()) + 2*borderMargin, (int)(rect.height()) + 2*borderMargin, QImage::Format_RGB32);
#endif

    QPainter painter(&img);
    painter.fillRect(img.rect(), QColor(250,250,250));
    auto borderRect = img.rect().adjusted(borderMargin,borderMargin, -borderMargin, -borderMargin);
    scene()->render(&painter, borderRect);

    painter.drawRect(borderRect);

    auto metrics = painter.fontMetrics();
    QString author("created by loginsight");
    painter.setPen(Qt::gray);
    QString site("https://github.com/compilelife/loginsight");
    auto width = metrics.horizontalAdvance(site);
    auto x = img.width() - width - 2*borderMargin;
    auto y = img.height() - metrics.height() - 2*borderMargin;
    painter.drawText(x, y, author);
    painter.drawText(x, y+metrics.height(), site);

    handler(img);
}
