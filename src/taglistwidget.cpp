#include "taglistwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QDebug>
#include "choosecolormenu.h"
#include <QJsonArray>

TagListWidget::TagListWidget()
{
    setFlow(QListView::LeftToRight);
    setWrapping(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setResizeMode(QListView::Adjust);
    setSpacing(2);
    setStyleSheet("background-color: transparent");

    setContextMenuPolicy(Qt::DefaultContextMenu);
}

void TagListWidget::contextMenuEvent(QContextMenuEvent *event)
{
    auto item = itemAt(event->x(), event->y());
    if (!item)
        return;

    auto keyword = item->data(Qt::DisplayRole).toString();

    QMenu menu;
    connect(menu.addAction("删除"), &QAction::triggered, [&]{
        takeItem(row(item));
        emit onTagDeleted(keyword);
    });

    auto chooseColorMenu = new ChooseColorMenu({{"红色",Qt::red},{"绿色",Qt::green},{"蓝色",Qt::blue}});
    connect(chooseColorMenu, &ChooseColorMenu::chooseColor, [&](QColor color){
        item->setData(Qt::DecorationRole, color);
        emit onTagColorChanged(keyword, color);
    });
    menu.addMenu(chooseColorMenu);

    connect(menu.addAction("在小窗过滤"), &QAction::triggered, [&]{
        Filter filter;
        filter.keyword = keyword;
        emit requestFilterTag(filter);
    });

    connect(menu.addAction("搜索"), &QAction::triggered, [&]{
        emit requestSearchTag(keyword);
    });


    menu.exec(event->globalPos());
}

void TagListWidget::addTag(QString keyword, QColor color)
{
    auto item = new QListWidgetItem();
    addItem(item);

    item->setData(Qt::DisplayRole,keyword);
    item->setData(Qt::DecorationRole, color);
}
