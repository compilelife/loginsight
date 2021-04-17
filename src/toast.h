#ifndef TOAST_H
#define TOAST_H

#include <QObject>
#include <QRect>
class ToastDlg;

class Toast: QObject
{
public:
    enum Level{
      INFO, WARN, ERROR
    };
private:
    Toast();
public:
    static Toast& instance();
public:
    void show(Level level, const QString& text);

private:
    void timerEvent(QTimerEvent *event) override;

private:
    ToastDlg* mDlg;
    int mTimerId{0};
    QRect mGeometry;
};

#endif // TOAST_H
