#include "toast.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>

class ToastDlg: public QDialog
{
private:
    QLabel* mLabel;
    QLabel* mCloseBtn;
protected:
    bool eventFilter(QObject *obj, QEvent *ev) override {
        if (obj == mCloseBtn) {
            if (ev->type() == QEvent::MouseButtonRelease) {
                accept();
            }
        }
        return QObject::eventFilter(obj, ev);
    }
public:
    ToastDlg() {
        auto layout = new QHBoxLayout;

        mLabel = new QLabel;
        mLabel->setStyleSheet("color: white; background:transparent");
        layout->addWidget(mLabel, 1);

        mCloseBtn = new QLabel;
        mCloseBtn->setPixmap(QPixmap(":/res/img/close.png"));
        mCloseBtn->installEventFilter(this);
        mCloseBtn->setStyleSheet("background:transparent");
        layout->addWidget(mCloseBtn);

        setLayout(layout);
        setWindowFlag(Qt::FramelessWindowHint);
        setAttribute(Qt::WA_ShowWithoutActivating,true);
    }

    void show(Toast::Level level, const QString& text) {
        QPalette p = palette();
        p.setColor(QPalette::Window, QColor(0,0,0,200));
        if (level == Toast::INFO) {
            p.setColor(QPalette::Window, QColor(0,0,0,200));
        } else if (level == Toast::WARN) {
            p.setColor(QPalette::Window, QColor(0,0,255,200));
        } else {
            p.setColor(QPalette::Window, QColor(255,0,0,200));
        }
        setPalette(p);

        mLabel->setText(text);
        setWindowFlag(Qt::WindowStaysOnTopHint);
        QDialog::show();
    }
};

Toast::Toast()
    :mDlg(new ToastDlg)
{
}

Toast &Toast::instance()
{
    static Toast thiz;
    return thiz;
}

void Toast::show(Toast::Level level, const QString &text)
{
    mDlg->show(level, text);
    if (mTimerId != 0) {
        killTimer(mTimerId);
    }
    mTimerId = startTimer(2000);
}

void Toast::timerEvent(QTimerEvent *event)
{
    killTimer(mTimerId);
    mTimerId = 0;
    mDlg->accept();
}
