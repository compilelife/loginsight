#ifndef FEEDBACKDLG_H
#define FEEDBACKDLG_H

#include <QDialog>

namespace Ui {
class FeedbackDlg;
}

class FeedbackDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FeedbackDlg(QWidget *parent = nullptr);
    ~FeedbackDlg();

private:
    Ui::FeedbackDlg *ui;
};

#endif // FEEDBACKDLG_H
