#include "choosecolormenu.h"
#include <QColorDialog>

ChooseColorMenu::ChooseColorMenu(const QList<pair<QString,QColor>>& preset)
    :QMenu()
{
    setTitle("设置颜色");

    for (int i = 0; i < preset.size(); i++) {
        auto name = preset[i].first;
        auto color = preset[i].second;
        QPixmap pixmap(10,10);
        pixmap.fill(color);
        connect(addAction(QIcon(pixmap), name), &QAction::triggered, [this,color]{
           emit chooseColor(color);
        });
    }

    connect(addAction("更多..."), &QAction::triggered, [this]{
        auto color = QColorDialog::getColor();
        if (color.isValid()) {
            emit chooseColor(color);
        }
    });
}
