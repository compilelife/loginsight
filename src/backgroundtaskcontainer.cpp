#include "backgroundtaskcontainer.h"
#include <QDebug>

BackgroundTaskContainer::BackgroundTaskContainer(BackgroundTask task, LongtimeOperation& op)
    :mTask(task),mOp(op)
{

}

void BackgroundTaskContainer::runTask()
{
    mTask(mOp);
    emit taskDone();
}
