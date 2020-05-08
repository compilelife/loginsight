#include "backgroundrunner.h"
#include <QEventLoop>
#include <memory>
#include <QDebug>

using namespace std;

#define TIMER_STOP_FOR_TIMEOUT 0
#define TIMER_STOP_FOR_DONE 1

BackgroundRunner::BackgroundRunner()
{
    mThread.start();
}

BackgroundRunner::~BackgroundRunner()
{
    mThread.quit();
    mThread.wait(1000);
}

BackgroundRunner &BackgroundRunner::instance()
{
    static BackgroundRunner inst;
    return inst;
}

bool BackgroundRunner::exec(const QString& name, BackgroundTask task)
{
    mOp.terminate = false;

    unique_ptr<BackgroundTaskContainer> taskContainer(new BackgroundTaskContainer(task, mOp));
    auto container = taskContainer.get();
    container->moveToThread(&mThread);
    connect(this, SIGNAL(beginTask()), container, SLOT(runTask()));
    connect(container, SIGNAL(taskDone()), this, SLOT(taskDone()));

    //任务可能在500ms内执行完成，这样taskDone会以TIMER_STOP_FOR_DONE退出mLoop
    //也可能在500ms外执行完成，这样timerEvent会以TIMER_STOP_FOR_TIMEOUT退出mLoop
    emit beginTask();
    mTimerId = startTimer(500);//singleshot

    int ret = mLoop.exec(QEventLoop::ExcludeUserInputEvents);//looper用来保证界面不会未响应
    if (ret == TIMER_STOP_FOR_DONE) {//if 已经执行完成，直接退出
        return false;
    }

    //else 还没执行完，就弹出一个进度框
    mDlg.setWindowTitle(name);
    return mDlg.exec() == QDialog::Rejected;
}

void BackgroundRunner::timerEvent(QTimerEvent *)
{
    killTimer(mTimerId);
    mTimerId = 0;
    mLoop.exit(TIMER_STOP_FOR_TIMEOUT);
}

void BackgroundRunner::taskDone()
{
    killTimer(mTimerId);
    mTimerId = 0;
    mLoop.exit(TIMER_STOP_FOR_DONE);
    mDlg.finish();
}
