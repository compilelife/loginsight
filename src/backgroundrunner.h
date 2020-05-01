#ifndef BACKGROUNDRUNNER_H
#define BACKGROUNDRUNNER_H

#include <QObject>
#include "LongtimeOperation.h"
#include <QEventLoop>
#include <QThread>
#include "progressdialog.h"
#include "backgroundtaskcontainer.h"

class BackgroundRunner:QObject
{
    Q_OBJECT

private:
    BackgroundRunner();
    ~BackgroundRunner();

public:
    static BackgroundRunner& instance();

public:
    bool exec(const QString& name, BackgroundTask task);

protected:
    void timerEvent(QTimerEvent *event) override;

private slots:
    void taskDone();

signals:
    void beginTask();

private:
    LongtimeOperation mOp;
    QEventLoop mLoop;
    QThread mThread;
    ProgressDialog mDlg{mOp};
    int mTimerId;
};

#endif // BACKGROUNDRUNNER_H
