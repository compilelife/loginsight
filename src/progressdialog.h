#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include "LongtimeOperation.h"

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(LongtimeOperation& op, QWidget *parent = nullptr);
    ~ProgressDialog() override;
public:
    int exec() override;
protected:
    void timerEvent(QTimerEvent *event) override;
    void closeEvent(QCloseEvent *) override;
private:
    void updateValue();
private:
    Ui::ProgressDialog *ui;
    LongtimeOperation& mOp;
    int mTimerId;
};

#endif // PROGRESSDIALOG_H
