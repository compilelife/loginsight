#include "logtextedit.h"
#include <QFile>
#include <QDebug>
#include <string>
#include <QScrollBar>
#include <QFont>
#include <QTextBlock>
#include <QPainter>
#include <QMenu>
#include "highlighter.h"
#include "backgroundrunner.h"
#include <QSettings>
#include <QDropEvent>
#include <QMimeData>


using namespace std;

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(LogTextEdit *editor) : QWidget(editor) {
        mEditor = editor;
    }

    QSize sizeHint() const override {
        return QSize(mEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        mEditor->lineNumberAreaPaintEvent(event);
    }

private:
    LogTextEdit *mEditor;
};

LogTextEdit::LogTextEdit(QWidget* parent)
    :QPlainTextEdit(parent)
{
    setLineWrapMode(QPlainTextEdit::NoWrap);

    QFont font;
    QSettings setting;
    font.setFamily(setting.value("editorFont").toString());
    font.setPointSize(setting.value("editorFontSize", 12).toInt());
    setFont(font);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(handleInternalScroll(int)));
    connect(this, SIGNAL(textChanged()), this, SLOT(undo()));//保持光标可见的同时模拟readonly

    mLineNumArea = new LineNumberArea(this);
    connect(this, &LogTextEdit::blockCountChanged, this, &LogTextEdit::updateLineNumberAreaWidth);
    connect(this, &LogTextEdit::updateRequest, this, &LogTextEdit::updateLineNumberArea);

    updateLineNumberAreaWidth(0);

    connect(this, &LogTextEdit::cursorPositionChanged, this, &LogTextEdit::highlightCurrentLine);
    highlightCurrentLine();

    mHighlighter = new Highlighter(document());
}

LogTextEdit::~LogTextEdit()
{
    delete mHighlighter;
}

void LogTextEdit::setLog(Log *log)
{
    mLog = log;
    mHistory.clear();

    if (log == nullptr) {
        mFromLine = mToLine = 0;
        clear();
        return;
    }

    load(1);

    mExternalBar->setRange(1, mLog->lineCount() - mViewPortLineCnt);

    emit beenFocused(this);
}

void LogTextEdit::setScrollBar(QScrollBar *scrollbar)
{
    mExternalBar = scrollbar;
    connect(mExternalBar, SIGNAL(valueChanged(int)), this, SLOT(handleExternalScroll(int)));
}

void LogTextEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    auto metrics = fontMetrics();
    mViewPortLineCnt = height()/metrics.height();

    QRect cr = contentsRect();
    mLineNumArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void LogTextEdit::dropEvent(QDropEvent *e)
{
    if (!e->mimeData()->hasUrls()) {
        return;
    }

    emit userDropFile(e->mimeData()->urls()[0].path());
}

void LogTextEdit::load(int topLine)
{
    mLoading = true;

    //从中间行向上向下拓宽mPreloadLineCnt / 2来加载，这样可以保证滚动到一半，反向滚动的时候无需触发加载
    //即，会让窗口可视区处于新加载后内容的中央位置
    auto centerLine = topLine + mViewPortLineCnt / 2;
    mFromLine = centerLine - (mPreloadLineCnt / 2);
    if (mFromLine < 1)
        mFromLine = 1;
    mToLine = mFromLine + mPreloadLineCnt - 1;
    if (mToLine > mLog->lineCount())
        mToLine = mLog->lineCount();

    auto horiValue = horizontalScrollBar()->value();
    setPlainText(mLog->getLine(mFromLine, mToLine));
    horizontalScrollBar()->setValue(horiValue);

    verticalScrollBar()->setValue(fromLogToViewPort(topLine));

    mLoading = false;
}

void LogTextEdit::handleInternalScroll(int)
{
    if (mLoading)
        return;

    mScrolling = true;
    auto topLine = fromViewPortToLog(firstVisibleBlock().blockNumber());

    mExternalBar->setValue(topLine);

    //window的作用是在还没消耗完预加载内容的时候就触发加载
    auto windowTop = topLine - 5;
    if (windowTop < 1)
        windowTop = 1;
    auto windowBottom = topLine + mViewPortLineCnt + 5;
    if (windowBottom > mLog->lineCount()) {
        windowBottom = mLog->lineCount();
    }

//    qDebug()<<windowTop<<","<<windowBottom<<";"<<mFromLine<<","<<mToLine;

    if (!(mFromLine<=windowTop && mToLine>=windowBottom)) {//window滑出预加载区了
        load(topLine);
    }
}

void LogTextEdit::handleExternalScroll(int value)
{
    auto viewportTopLine = fromLogToViewPort(value);
    if (viewportTopLine == verticalScrollBar()->value()) {
        return;
    }

    auto min = verticalScrollBar()->minimum();
    auto max = verticalScrollBar()->maximum();
    if (viewportTopLine >= min && viewportTopLine <= max) {
        mLoading = true;
        verticalScrollBar()->setValue(viewportTopLine);
        mLoading = false;
    } else {
        load(value);
    }
}

int LogTextEdit::fromViewPortToLog(int index)
{
    //返回index从1开始
    return index + mFromLine;
}

int LogTextEdit::fromLogToViewPort(int line)
{
    //返回index从0开始
    return  line - mFromLine;
}

int LogTextEdit::centerLineGetTopRow(int line)
{
    return line - mViewPortLineCnt/2;
}

void LogTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(mLineNumArea);
    QColor bgColor(230,230,230);
    QColor fgColor(Qt::gray);
    if (mFocused) {
        bgColor.setRgb(0, 153, 204);
        fgColor.setRgb(255, 255, 255);
    }
    painter.fillRect(event->rect(), bgColor);

    QTextBlock block = firstVisibleBlock();
    int lineNumber = fromViewPortToLog(block.blockNumber());

    int y = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    painter.setPen(fgColor);
    while (block.isValid() && y <= event->rect().bottom()) {
        if (block.isVisible()) {
            painter.drawText(0, y, mLineNumArea->width(), fontMetrics().height(),
                             Qt::AlignCenter, QString::number(lineNumber));
        }

        y += (int)blockBoundingRect(block).height();
        block = block.next();
        ++lineNumber;
    }
}

int LogTextEdit::lineNumberAreaWidth()
{
    int digitCnt = 1;
    int max = mToLine;
    while (max >= 10) {
        max /= 10;
        ++digitCnt;
    }

    //左右各留2个像素
    int space = 4 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digitCnt;

    return space;
}

void LogTextEdit::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void LogTextEdit::updateLineNumberArea(const QRect & rect, int dy)
{
    if (dy)
        mLineNumArea->scroll(0, dy);
    else
        mLineNumArea->update(0, rect.y(), mLineNumArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void LogTextEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setTextOutline(QPen(Qt::black));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void LogTextEdit::addWordHighlightUnderCursor()
{
    auto cursor = textCursor();

    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    mHighlighter->quickHighlight(cursor.selectedText());
}

bool LogTextEdit::search(const QString &text, QTextDocument::FindFlags options)
{
    auto cursor = textCursor();
    auto line = fromViewPortToLog(cursor.blockNumber());

    //清除选区以保证连续search的正确
    if (cursor.hasSelection()) {
        auto len = cursor.selectionEnd() - cursor.selectionStart();
        if (options.testFlag(QTextDocument::FindBackward)) {
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, len);
        } else {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, len);
        }
    }

    SearchResult target;
    bool canceled = BackgroundRunner::instance().exec(QString("查找%1").arg(text), [&](LongtimeOperation& op){
        target = mLog->search(text, options, line, cursor.position() - cursor.block().position(), op);
    });

    if (canceled) {
        return true;
    }

    if (target.line <= 0) {
        return false;
    }

    mHighlighter->clearSearchHighlight();

    scrollToLine(target.line, target.pos);//将target行滚动到窗口中间

    //利用select选择search词
    cursor = textCursor();
    if (options.testFlag(QTextDocument::FindBackward)) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, text.length());
    } else {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, text.length());
    }
    setTextCursor(cursor);

    mHighlighter->searchHighlight(text, options.testFlag(QTextDocument::FindCaseSensitively));
    repaint();

    return true;
}

void LogTextEdit::handleFilterUnderCursor()
{
    auto cursor = textCursor();

    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    emit requestFilter(cursor.selectedText());
}

void LogTextEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu* menu = new QMenu();

    auto cursor = textCursor();
    mMenuCursor = cursorForPosition(e->pos());
    mMenuCursor.select(QTextCursor::WordUnderCursor);

    {
        auto action = menu->addAction("添加到时间线");
        connect(action, SIGNAL(triggered()), this, SLOT(handleMarkLineAction()));
    }

    menu->addSeparator();
    auto cursorWord = cursor.selectedText();

    if (cursorWord.length() > 20) {
        cursorWord = cursorWord.mid(0, 20) + "...";
    }

    if (cursorWord.length() > 0 ) {
        auto action = menu->addAction(QString("高亮\"%1\"").arg(cursorWord));
        connect(action, SIGNAL(triggered()), this, SLOT(addWordHighlightUnderCursor()));
        action = menu->addAction(QString("过滤含\"%1\"的行").arg(cursorWord));
        connect(action, SIGNAL(triggered()), this, SLOT(handleFilterUnderCursor()));

        action = menu->addAction("清除搜索高亮");
        connect(action, SIGNAL(triggered()), mHighlighter, SLOT(clearSearchHighlight()));
    }
    else {
        menu->addAction("请选择文本来高亮、过滤和搜索")->setEnabled(false);
    }

    if (cursorWord.length() > 0) {
        menu->addSeparator();
        connect(menu->addAction("复制"), SIGNAL(triggered()), this, SLOT(copy()));
    }

    menu->exec(e->globalPos());
    delete menu;
}

void LogTextEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    auto cursor = cursorForPosition(e->pos());
    emit emphasizeLine(fromViewPortToLog(cursor.blockNumber()));
}

void LogTextEdit::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        emit returnPressed();
    } else if (e->key() == Qt::Key_PageDown || e->key() == Qt::Key_PageUp) {
        handleInternalScroll(0);
    }
    QPlainTextEdit::keyReleaseEvent(e);
}

void LogTextEdit::refresh()
{
    auto hlLine = fromViewPortToLog(textCursor().blockNumber());
    auto topLine = fromViewPortToLog(firstVisibleBlock().blockNumber());

    load(topLine);

    auto cursor = textCursor();
    auto block = document()->findBlockByLineNumber(fromLogToViewPort(hlLine));
    cursor.setPosition(block.position());

    setTextCursor(cursor);
}

void LogTextEdit::focusInEvent(QFocusEvent *e)
{
    emit beenFocused(this);
    QPlainTextEdit::focusInEvent(e);
}

void LogTextEdit::setVisible(bool visible)
{
    QPlainTextEdit::setVisible(visible);
    mExternalBar->setVisible(visible);
    setEnabled(visible);
}

void LogTextEdit::scrollToLine(int lineNum, int col, bool recordToHistory)
{
    auto cursor = textCursor();
    if (recordToHistory)
        mHistory.push(fromViewPortToLog(cursor.blockNumber()), cursor.columnNumber());

    handleExternalScroll(centerLineGetTopRow(lineNum));//将target行滚动到窗口中间

    auto viewPortTarget = fromLogToViewPort(lineNum);
    auto block = document()->findBlockByLineNumber(viewPortTarget);
    auto pos = block.position() + col;

    cursor.setPosition(pos);
    setTextCursor(cursor);

    if (recordToHistory)
        mHistory.push(lineNum, col);
    repaint();
}

void LogTextEdit::handleMarkLineAction()
{
    auto s = mMenuCursor.block().text();
    auto line = fromViewPortToLog(mMenuCursor.blockNumber());
    emit requestMarkLine(line, s);
}

void LogTextEdit::onReplay(const NavPos &pos)
{
    scrollToLine(pos.line, pos.col, false);
}

void LogTextEdit::drawFocused()
{
    mFocused = true;
    repaint();
}

void LogTextEdit::drawUnFocused()
{
    mFocused = false;
    repaint();
}
