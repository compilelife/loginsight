#ifndef LINEHIGHLIGHTER_H
#define LINEHIGHLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QQuickTextDocument>
#include "textcodec.h"
#include <utility>

using namespace std;


class LineHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit LineHighlighter(QSyntaxHighlighter *parent = nullptr);

    Q_INVOKABLE void setup(QQuickTextDocument* document);

    Q_PROPERTY(QVector<QString> segColors MEMBER segColors)
    Q_PROPERTY(QVector<QVariantMap> highlights MEMBER highlights)
    Q_PROPERTY(QVector<QVariantMap> segs MEMBER segs)
    Q_PROPERTY(QVariantMap searchResult MEMBER searchResult)
    Q_PROPERTY(TextCodec* textCodec MEMBER textCodec)

private:
    pair<int,int> fromLogOffsetLen(const QString &text, int offset, int length);

private:
    QVector<QVariantMap> segs;
    QVector<QString> segColors;
    QVector<QVariantMap> highlights;
    QVariantMap searchResult;
    TextCodec* textCodec;

protected:
    void highlightBlock(const QString &text) override;

};

#endif // LINEHIGHLIGHTER_H
