#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QProgressDialog>
#include "longtimeoperation.h"
#include <memory>
#include <functional>

using namespace std;

class ProgressDialog : public QProgressDialog
{
public:
    ProgressDialog(shared_ptr<LongtimeOperation> op, const QString &hint, function<void()> cleanup);

public:
    void timerEvent(QTimerEvent *event) override;
private:
    shared_ptr<LongtimeOperation> mOp;
    int mTimerId{-1};
};

#endif // PROGRESSDIALOG_H
