#ifndef TAGLISTWIDGET_H
#define TAGLISTWIDGET_H

#include <QListWidget>
#include "highlighter.h"

class TagListWidget : public QListWidget
{
    Q_OBJECT

public:
    TagListWidget();

signals:
    void onTagDeleted(int index);
    void onTagSelected(int index);
    void onTagColorChanged(int index, QColor color);
private:
    void setBgColor(QWidget* w, QColor color);
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

public:
    void addTag(QString keyword, QColor color);
};

#endif // TAGLISTWIDGET_H
