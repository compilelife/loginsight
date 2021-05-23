#include "progressdialog.h"
#include <QDebug>

ProgressDialog::ProgressDialog(shared_ptr<LongtimeOperation> op, const QString &hint, function<void()> cleanup)
{
    mOp = op;

    setModal(true);

    setMaximum(op->getProgressMax());
    setMinimum(op->getProgressMin());

    setMinimumDuration(1000);//大于2s的操作才显示

    setLabelText(hint);
    setAutoClose(true);

    connect(this, &QProgressDialog::canceled, [this]{
        mOp->cancel();
        finished(1);
    });

    connect(this, &QProgressDialog::finished, [this, cleanup]{
        qDebug()<<"finished";

        cleanup();
        killTimer(mTimerId);
        this->deleteLater();
    });

    setValue(minimum());

    mTimerId = startTimer(50);//每50ms更新一次进度
    mOp->start();
}

void ProgressDialog::timerEvent(QTimerEvent *e) {
    QProgressDialog::timerEvent(e);

//    qDebug()<<"set value:"<<mOp->getProgressValue();
    setValue(mOp->getProgressValue());
    if (mOp->getState() == LongtimeOperation::eStateDone)
        finished(0);
}

