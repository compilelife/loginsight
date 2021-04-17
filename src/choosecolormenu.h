#ifndef CHOOSECOLORMENU_H
#define CHOOSECOLORMENU_H

#include <QMenu>
#include <QList>
#include <QColor>
#include <utility>

using namespace std;

class ChooseColorMenu : public QMenu
{
    Q_OBJECT

public:
    ChooseColorMenu(const QList<pair<QString,QColor>>& preset);

signals:
    void chooseColor(QColor color);
};

#endif // CHOOSECOLORMENU_H
