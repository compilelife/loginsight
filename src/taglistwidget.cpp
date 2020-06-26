#include "taglistwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QDebug>
#include "choosecolormenu.h"

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

void TagListWidget::setBgColor(QWidget *w, QColor color)
{
    auto bgColor = QString("rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
    auto css = QString("background: %1; border-radius:2px;color:white;").arg(bgColor);
    w->setStyleSheet(css);
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
        setBgColor(itemWidget(item), color);
        emit onTagColorChanged(keyword, color);
    });
    menu.addMenu(chooseColorMenu);

    connect(menu.addAction("在小窗过滤"), &QAction::triggered, [&]{
        emit requestFilterTag(keyword);
    });

    connect(menu.addAction("搜索"), &QAction::triggered, [&]{
        emit requestSearchTag(keyword);
    });


    menu.exec(event->globalPos());
}

void TagListWidget::addTag(QString keyword, QColor color)
{
    auto w = new QWidget;
    auto box = new QHBoxLayout();
    box->setMargin(2);

    auto label = new QLabel(QString("%1").arg(keyword));
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    box->addWidget(label);

    w->setLayout(box);
    setBgColor(w, color);

    auto size = w->sizeHint();

    auto item = new QListWidgetItem();
    item->setSizeHint(size);
    item->setData(Qt::DisplayRole, keyword);
    addItem(item);
    setItemWidget(item, w);
}
