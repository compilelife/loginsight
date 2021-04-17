#ifndef RECENT_H
#define RECENT_H

#include <QString>
#include <QMenu>
#include <functional>

class Recent
{
public:
    Recent(QString key, int maxCount);

    void add(QString path);

    void mount(QMenu* menu, std::function<void(QString path)> func);

private:
    void createActions();

private:
    QString mKey;
    int mMaxCount;
    QMenu* mMenu;
    std::function<void(QString path)> mFunc;
};

#endif // RECENT_H
