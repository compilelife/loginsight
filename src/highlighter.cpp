#include "highlighter.h"

Highlighter::Highlighter(QTextDocument* doc): QSyntaxHighlighter(doc) {
}

void Highlighter::highlightBlock(const QString &text) {
    bool searchHlApplied = false;

    //选词高亮
    for (auto p : mQuickHls) {
        int i = 0;
        auto len = p.key.length();
        QTextCharFormat fmt;
        fmt.setForeground(Qt::white);
        fmt.setBackground(p.color);

        if (!searchHlApplied && p.key == mSearchHl.key) {
            fmt.setFontUnderline(true);
            searchHlApplied=true;
        }

        while ((i = text.indexOf(p.key, i, p.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) >= 0) {
            setFormat(i, len, fmt);
            i+=len;
        }
    }

   //搜索高亮
    if (mSearchHl.key.length() == 0 || searchHlApplied)
        return;

    auto len = mSearchHl.key.length();
    QTextCharFormat fmt;
    fmt.setFontUnderline(true);
    int i = 0;
    while ((i = text.indexOf(mSearchHl.key, i, mSearchHl.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) >= 0) {
        setFormat(i, len, fmt);
        i+=len;
    }
}

void Highlighter::quickHighlight(const QString &text, QColor color) {
    if (mQuickHls.contains(text)) {
        if (mQuickHls[text].color != color) {
            mQuickHls[text].color = color;
            rehighlight();
            return;
        }
    }

    HighlightPattern pattern;
    pattern.key = text;
    pattern.color = color;
    pattern.caseSensitive = true;

    mQuickHls.insert(text, pattern);

    rehighlight();

    emit onPatternAdded(pattern);
}

void Highlighter::searchHighlight(const QString &text, bool caseSensitive)
{
    mSearchHl.key = text;
    mSearchHl.caseSensitive = caseSensitive;

    rehighlight();
}

void Highlighter::clearSearchHighlight()
{
    mSearchHl.key.clear();
    rehighlight();
}

void Highlighter::clearHighlight()
{
    mQuickHls.clear();
    mSearchHl.key.clear();
    rehighlight();
}

void Highlighter::clearQuickHighlight(const QString& text)
{
    mQuickHls.remove(text);
    rehighlight();
}

