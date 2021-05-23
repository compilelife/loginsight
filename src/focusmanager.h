#ifndef FOCUSMANAGER_H
#define FOCUSMANAGER_H

#include <QWidget>
#include <QSet>

class FocusManager : public QObject{
    Q_OBJECT

public:
    FocusManager();
    ~FocusManager();

public:
    void add(QWidget* edit);
    void remove(QWidget* edit);
    bool isInFocus(QWidget *edit);
    QWidget* current();

signals:
    void focusChanged(QWidget* old, QWidget* now);

private:
    QSet<QWidget*> mItems;
    QWidget* mCurrent{nullptr};
    QMetaObject::Connection mConnectId;
};

#endif // FOCUSMANAGER_H
