#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>

struct HighlightPattern {
    QString key;
    bool caseSensitive{false};
    QColor color;
};

class Highlighter: public QSyntaxHighlighter
{
    Q_OBJECT
public:
    Highlighter(QTextDocument* doc);
protected:
    void highlightBlock(const QString &text) override;

public:
    void quickHighlight(const QString& text, QColor color = Qt::blue);
    void searchHighlight(const QString& text, bool caseSensitive);

    const QList<HighlightPattern> allQuickHighlights(){return mQuickHls.values();}

signals:
    void onPatternAdded(HighlightPattern pattern);

public slots:
    void clearQuickHighlight(const QString& text);
    void clearSearchHighlight();
    void clearHighlight();

private:
    QMap<QString, HighlightPattern> mQuickHls;
    HighlightPattern mSearchHl;
};

#endif // HIGHLIGHTER_H
