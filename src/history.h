#ifndef HISTORY_H
#define HISTORY_H

#include <QList>

struct NavPos {
    int line;
    int col;
};

class AbstractNavPosReplayer {
public:
    virtual ~AbstractNavPosReplayer();
    virtual void onReplay(const NavPos& pos) = 0;
};

class History: public QObject
{
    Q_OBJECT

public:
    History(AbstractNavPosReplayer* replayer, int max=50);

public:
    void clear();
    void foward();
    void backward();
    int availableBackwardCount();
    int availableFowardCount();
    void push(int lineNum, int col);

signals:
    void posChanged();

private:
    void setPos(int pos);

private:
    QList<NavPos> mItems;
    int mCur;
    const int mMax;
    AbstractNavPosReplayer* mReplayer;
};

#endif // HISTORY_H
