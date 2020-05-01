#ifndef BACKGROUNDTASKCONTAINER_H
#define BACKGROUNDTASKCONTAINER_H

#include <QObject>
#include <functional>
#include "LongtimeOperation.h"
using namespace std;

using BackgroundTask = function<void(LongtimeOperation&)>;

//wrap BackgroundTask，方便move到线程执行
class BackgroundTaskContainer: public QObject
{
    Q_OBJECT

public:
    BackgroundTaskContainer(BackgroundTask task, LongtimeOperation& op);

public slots:
    void runTask();

signals:
    void taskDone();

private:
    BackgroundTask mTask;
    LongtimeOperation& mOp;
};

#endif // BACKGROUNDTASKCONTAINER_H
