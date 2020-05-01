#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(LongtimeOperation& op, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog),
    mOp(op)
{
    ui->setupUi(this);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

int ProgressDialog::exec()
{
    updateValue();
    mTimerId = startTimer(500);
    return QDialog::exec();
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
    auto value = (mOp.cur - mOp.from) * 1.0f / (mOp.to - mOp.from) * 100;
    ui->progressBar->setValue(value);

    ui->label->setText(QString("%1 / %2 - %3  (%4%)").arg(mOp.cur).arg(mOp.from).arg(mOp.to).arg(ui->progressBar->value()));
}
