#include "recent.h"
#include <QSettings>

Recent::Recent(QString key, int maxCount)
    :mKey(key), mMaxCount(maxCount)
{

}

void Recent::add(QString path)
{
    QSettings config;
    auto lst = config.value(mKey).toList();

    if (lst.contains(path)) {
        lst.removeOne(path);
    }

    lst.push_front(path);
    if (lst.size() > mMaxCount) {
        lst.pop_back();
    }

    config.setValue(mKey, lst);
    config.sync();

    createActions();
}

void Recent::mount(QMenu *menu, std::function<void (QString)> func)
{
    mMenu = menu;
    mFunc = func;
    createActions();
}

void Recent::createActions()
{
    QSettings config;
    auto lst = config.value(mKey).toList();

    mMenu->clear();

    for (auto& i : lst) {
        auto path = i.toString();
        mMenu->addAction(path, [this, path]{
            mFunc(path);
        });
    }

    mMenu->addAction("清除", [this]{
        QSettings config;
        config.setValue(mKey, QStringList{});
        config.sync();
        createActions();
    });
}
