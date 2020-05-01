#include "highlighter.h"

Highlighter::Highlighter(QTextDocument* doc): QSyntaxHighlighter(doc) {
    mQuickHl.color.setRgb(0,153,0);
    mSearchHl.color.setRgb(255,0,0);
}

void Highlighter::highlightBlock(const QString &text) {
    for (auto p : mPatterns) {
        int i = 0;
        auto len = p->key.length();
        QTextCharFormat fmt;
        fmt.setForeground(p->color);

        while ((i = text.indexOf(p->key, i, p->caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) >= 0) {
            setFormat(i, len, fmt);
            i+=len;
        }
    }
}

void Highlighter::quickHighlight(const QString &text) {
    mQuickHl.key = text;
    mPatterns.insert(&mQuickHl);
    rehighlight();
}

bool Highlighter::isQuickHighlightOn() {
    return mPatterns.contains(&mQuickHl);
}

void Highlighter::searchHighlight(const QString &text, bool caseSensitive)
{
    mSearchHl.key = text;
    mSearchHl.caseSensitive = caseSensitive;
    mPatterns.insert(&mSearchHl);
    rehighlight();
}

bool Highlighter::isSearchHighlightOn() {
    return mPatterns.contains(&mSearchHl);
}

QString Highlighter::getQuickText()
{
    return mQuickHl.key;
}

QString Highlighter::getSearchText()
{
    return mSearchHl.key;
}

void Highlighter::clearQuickHighlight() {
    mPatterns.remove(&mQuickHl);
    rehighlight();
}

void Highlighter::clearSearchHighlight()
{
    mPatterns.remove(&mSearchHl);
    rehighlight();
}

void Highlighter::clearHighlight()
{
    mPatterns.clear();
    rehighlight();
}
