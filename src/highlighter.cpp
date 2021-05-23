#include "highlighter.h"
#include <QJsonObject>
#include <QJsonArray>

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

QJsonObject Highlighter::saveToJson()
{
    QJsonObject o;
    o["searchHl"] = mSearchHl.saveToJson();
    QJsonArray quickHls;
    for (auto&& v : mQuickHls.values()) {
        quickHls.append(v.saveToJson());
    }
    o["quickHls"] = quickHls;
    return o;
}

void Highlighter::loadFromJson(QJsonObject o)
{
    mSearchHl.loadFromJson(o["searchHl"].toObject());
    for (auto&& v : o["quickHls"].toArray()) {
        HighlightPattern p;
        p.loadFromJson(v.toObject());
        mQuickHls[p.key] = p;
    }
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


QJsonObject HighlightPattern::saveToJson()
{
    QJsonObject o;
    o["key"] = key;
    o["caseSentive"] = caseSensitive;
    o["color"] = color.name();
    return o;
}

void HighlightPattern::loadFromJson(QJsonObject o)
{
    key = o["key"].toString();
    caseSensitive = o["caseSentive"].toBool();
    color = o["color"].toString();
}
