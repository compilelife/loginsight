#include "linehighlighter.h"
#include <QDebug>
#include "textcodec.h"

LineHighlighter::LineHighlighter(QSyntaxHighlighter *parent)
    : QSyntaxHighlighter{parent}
{

}

void LineHighlighter::setup(QQuickTextDocument *document)
{
    setDocument(document->textDocument());
}

pair<int,int> LineHighlighter::fromLogOffsetLen(const QString &text, int offset, int length) {
    auto logBytes = textCodec->toLog(text);
    auto keyword = textCodec->toVisual(logBytes.mid(offset, length));
    auto keywordLength = keyword.length();
    auto suffix = textCodec->toVisual(logBytes.mid(offset));
    auto index = text.lastIndexOf(suffix);
    return {index, keywordLength};
}

void LineHighlighter::highlightBlock(const QString &text)
{
    if (text.isEmpty())
        return;

    //segs
    auto n = qMin(segs.size(), segColors.size());
    for (auto i = 0; i < n; i++) {
        auto& item = segs[i];
        auto ret = fromLogOffsetLen(text, item["offset"].toInt(), item["length"].toInt());

        QTextCharFormat fmt;
        fmt.setForeground(QColor(segColors[i]));
        setFormat(ret.first, ret.second, fmt);
    }

    //highlights
    for(auto&& item : highlights) {
        auto keyword = item["keyword"].toString();
        auto keywordLen = keyword.length();
        auto color = item["color"].toString();
        auto index = -1;

        QTextCharFormat fmt;
        fmt.clearForeground();//TODO：能否在设置背景的时候 ，根据背景色或保留前景色，或自动选择恰当反色？
        fmt.setBackground(QColor(color));

        do {
            index = text.indexOf(keyword, index+1);
            if (index >= 0)
                setFormat(index, keywordLen, fmt);
        }while(index >= 0);
    }

    //search result
    if (!searchResult.empty()) {
        auto ret = fromLogOffsetLen(text, searchResult["offset"].toInt(), searchResult["len"].toInt());

        QTextCharFormat fmt;
        fmt.clearBackground();
        fmt.setBackground(QColor(0,200,200));
        setFormat(ret.first, ret.second, fmt);
    }
}



