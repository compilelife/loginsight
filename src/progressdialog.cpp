#include "progressdialog.h"
#include "ui_progressdialog.h"
#include <QPushButton>

ProgressDialog::ProgressDialog(LongtimeOperation& op, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog),
    mOp(op)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint);
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(handleUserCancel()));
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

int ProgressDialog::exec()
{
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);
    mCanceled=false;
    mLast = 0;
    updateValue();
    mTimerId = startTimer(500);
    return QDialog::exec();
}

void ProgressDialog::finish()
{
    if (mCanceled)
        reject();
    else
        accept();
}

void ProgressDialog::handleUserCancel()
{
    mOp.terminate = true; //引起处理线程退出耗时操作
    mCanceled = true;//等待finish()被调用
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
}

void ProgressDialog::timerEvent(QTimerEvent *event)
{
    updateValue();
}

void ProgressDialog::closeEvent(QCloseEvent *)
{
    killTimer(mTimerId);
}

void ProgressDialog::updateValue()
{
    if (mOp.cur == mLast) {
        ++mOp.cur;//让进度条看起来至少有在动
    }
    mLast = mOp.cur;

    auto value = (mOp.cur - mOp.from) * 1.0f / (mOp.to - mOp.from) * 100;
    ui->progressBar->setValue(value);

    ui->label->setText(QString("%1 / %2 - %3  (%4%)").arg(mOp.cur).arg(mOp.from).arg(mOp.to).arg(ui->progressBar->value()));
}
