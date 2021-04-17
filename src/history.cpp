#include "history.h"

History::History(AbstractNavPosReplayer* replayer, int max)
    :mMax(max), mReplayer(replayer)
{
    clear();
}

void History::clear()
{
    mItems.clear();
    setPos(-1);
}

void History::foward()
{
    if (availableFowardCount() <= 0) {
        return;
    }

    setPos(mCur+1);

    mReplayer->onReplay(mItems.at(mCur));
}

void History::backward()
{
    if (availableBackwardCount() <= 0) {
        return;
    }

    setPos(mCur-1);

    mReplayer->onReplay(mItems.at(mCur));
}

int History::availableBackwardCount()
{
    return mCur;
}

int History::availableFowardCount()
{
    return mItems.size() - 1 - mCur;
}

void History::push(int lineNum, int col)
{
    if (mItems.size() > 0) {
        auto cur = mItems.at(mCur);
        if (cur.line == lineNum && cur.col == col) {
            return;
        }
    }

    //push要先清除mCur之后的记录（改变回溯栈）
    while (mItems.size() > mCur+1) {
        mItems.removeLast();
    }

    //限制到mMax
    while (mItems.size() >= mMax) {
        mItems.removeFirst();
    }

    mItems.push_back({lineNum, col});
    setPos(mItems.size() -1);
}

void History::setPos(int pos)
{
    mCur = pos;
    emit posChanged();
}

AbstractNavPosReplayer::~AbstractNavPosReplayer()
{

}
