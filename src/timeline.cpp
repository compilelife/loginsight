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
#include <QPropertyAnimation>
#include <QJsonArray>

TimeLine::TimeLine(QWidget* parent)
    :QGraphicsView(parent)
{
    setFrameStyle(QFrame::NoFrame);
    QGraphicsScene* scene = new QGraphicsScene;
    mWidth = 350;
    mHeight = 400;
    scene->setSceneRect(0,0,mWidth,mHeight);
    setScene(scene);

    mLineX = LINE_X+MARGIN_LEFT;
    mLineY = 8;
    const double d = 4;
    mLineHead = scene->addEllipse({(double)mLineX-d/2, 4, d, d}, Qt::NoPen, QColor(180,180,180));

    mLine = new QGraphicsLineItem();
    mLine->setLine(mLineX, mLineY, mLineX, mHeight);
    mLine->setPen(QPen(QColor(180,180,180)));
    scene->addItem(mLine);

    mNodeTop = 20;
}

void TimeLine::highlightNode(int lineNum)
{
    TimeNode* node = nullptr;
    for (int i = 0; i < mNodes.size(); i++) {
        if (lineNum == mNodes.at(i)->lineNumber()) {
            node = mNodes.at(i);
            break;
        }
    }

    if (node) {
        highlightItem(node);
    }
}

QJsonValue TimeLine::saveToJson()
{
    QJsonArray arr;
    for (auto& node : mNodes) {
        arr.push_back(node->saveToJson());
    }
    return arr;
}

void TimeLine::loadFromJson(const QJsonValue &jo)
{
    if (!jo.isArray())
        return;

    for (auto item : jo.toArray()) {
        addNode(new TimeNode(item));
    }
}

void TimeLine::addNode(int lineNum, const QString &text) {
    //找到插入位置
    int pos = mNodes.size();
    for (int i = 0; i < mNodes.size(); i++) {
        auto order = mNodes.at(i)->lineNumber();
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

    node->setX(MARGIN_LEFT);
    node->setY(calNodeY(pos));

    scene()->addItem(node);
    node->ensureVisible(0,0,10,10);

    connect(node, SIGNAL(requestDel(TimeNode*)), this, SLOT(deleteNode(TimeNode*)));
    connect(node, SIGNAL(selected(TimeNode*)), this, SIGNAL(nodeSelected(TimeNode*)));

    highlightItem(node);
}

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

void TimeLine::clear()
{
    for (auto* node : mNodes) {
        scene()->removeItem(node);
    }
    mNodes.clear();
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

void TimeLine::addNode(TimeNode *node)
{
    auto pos = mNodes.size();
    mNodes.insert(pos, node);

    fitLine();

    node->setX(MARGIN_LEFT);
    node->setY(calNodeY(pos));

    scene()->addItem(node);
    node->ensureVisible(0,0,10,10);

    connect(node, SIGNAL(requestDel(TimeNode*)), this, SLOT(deleteNode(TimeNode*)));
    connect(node, SIGNAL(selected(TimeNode*)), this, SIGNAL(nodeSelected(TimeNode*)));
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

    painter.setPen(Qt::gray);
    painter.drawRect(borderRect);

    auto metrics = painter.fontMetrics();
    QString author("created by loginsight");

    QString site("https://github.com/compilelife/loginsight");
    auto width = metrics.horizontalAdvance(site);
    auto x = img.width() - width - 2*borderMargin;
    auto y = img.height() - metrics.height() - 2*borderMargin;
    painter.drawText(x, y, author);
    painter.drawText(x, y+metrics.height(), site);

    handler(img);
}

void TimeLine::highlightItem(QGraphicsObject *item)
{
    auto anim = new QPropertyAnimation(item, "scale");
    anim->setDuration(500);
    anim->setKeyValueAt(0, 1.0);
    anim->setKeyValueAt(0.5, 1.2);
    anim->setKeyValueAt(1, 1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);

    scene()->clearSelection();
    item->setSelected(true);
}
