#ifndef LONGTIMEOPERATION_H
#define LONGTIMEOPERATION_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QVariant>
#include <QDebug>

//1. 发布进度
//2. 阻塞取消
class LongtimeOperation
{
public:
    enum State{
        eStatePause,
        eStateNormal,
        eStateCanceling,
        eStateDone
    };//pause->normal->cancling->done
    //               ->done
    //       ->done
    LongtimeOperation() {
    }

    void cancel() {
        QMutexLocker l(&mMutex);
        if (mState == eStateDone)
            return;

        mCond.notify_one();//可能当前有线程在waitStart

        mState = eStateCanceling;
        mCond.wait(&mMutex);
    }

    void start() {
        QMutexLocker l(&mMutex);
        if (mState != eStatePause)
            return;

        mState = eStateNormal;
        mCond.notify_all();
    }

    void waitStart() {
        QMutexLocker l(&mMutex);
        if (mState != eStatePause)
            return;
        mCond.wait(&mMutex);
    }

    int getProgressMax() {return mProgressMax;}
    int getProgressMin() {return mProgressMin;}
    int getProgressValue() {return mProgressValue;}

    void setProgressMax(int v) {mProgressMax = v;}
    void setProgressMin(int v) {mProgressMin = v;mProgressValue=v;}
    void publishProgress() {
        ++mProgressValue;
    }
    void setProgressValue(int v) {
        mProgressValue = v;
    }

    void done() {
        QMutexLocker l(&mMutex);
        mState = eStateDone;
        setProgressValue(getProgressMax());
        mCond.notify_all();
    }

    bool isCanceled() {
        return mState == eStateCanceling;
    }

    bool isDone() {
        return mState == eStateDone;
    }

    State getState() {
        return mState;
    }
private:
    State mState{eStatePause};

    int mProgressMax{1};
    int mProgressMin{0};
    int mProgressValue{0};

    QMutex mMutex;
    QWaitCondition mCond;
};

#endif // LONGTIMEOPERATION_H
