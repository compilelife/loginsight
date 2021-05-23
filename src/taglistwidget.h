#ifndef TAGLISTWIDGET_H
#define TAGLISTWIDGET_H

#include <QListWidget>
#include "highlighter.h"
#include <QJsonObject>
#include "ilog.h"

class TagListWidget : public QListWidget
{
    Q_OBJECT

public:
    TagListWidget();

signals:
    void onTagDeleted(const QString& keyword);
    void requestSearchTag(const QString& keyword);
    void onTagColorChanged(const QString& keyword, QColor color);
    void requestFilterTag(const SearchArg& filter);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

public:
    void addTag(QString keyword, QColor color);
};

#endif // TAGLISTWIDGET_H
