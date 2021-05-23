#ifndef HISTORY_H
#define HISTORY_H

#include <QList>
#include <QObject>
#include "ilog.h"

class AbstractNavPosReplayer {
public:
    virtual ~AbstractNavPosReplayer();
    virtual void onReplay(const Location& pos) = 0;
};

class History: public QObject
{
    Q_OBJECT

public:
    History(AbstractNavPosReplayer* replayer, int max=50);

public:
    void clear();
    void forward();
    void backward();
    int availableBackwardCount();
    int availableFowardCount();
    void push(int lineNum, int col);

signals:
    void posChanged();

private:
    void setPos(int pos);

private:
    QList<Location> mItems;
    int mCur;
    const int mMax;
    AbstractNavPosReplayer* mReplayer;
};

#endif // HISTORY_H
