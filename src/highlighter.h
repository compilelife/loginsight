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
    void quickHighlight(const QString& text);
    bool isQuickHighlightOn();

    void searchHighlight(const QString& text, bool caseSensitive);
    bool isSearchHighlightOn();

    QString getQuickText();
    QString getSearchText();

public slots:
    void clearQuickHighlight();
    void clearSearchHighlight();
    void clearHighlight();

private:
    HighlightPattern mQuickHl;
    HighlightPattern mSearchHl;
    QSet<HighlightPattern*> mPatterns;
};

#endif // HIGHLIGHTER_H
