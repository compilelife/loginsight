#include "logedit.h"
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QTimer>
#include <QApplication>
#include <QMessageBox>
#include "util.h"
#include <QSettings>

static const int PRELOAD_COUNT = 2560;//2560 200个字符一行，约500K

LogEdit::LogEdit(FocusManager* focusManager)
{
    mFocusManager = focusManager;
    mLna = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::textChanged, this, &QPlainTextEdit::undo);//保持光标可见的同时模拟readonly
    connect(this, &QPlainTextEdit::blockCountChanged, this, &LogEdit::updateLnaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &LogEdit::updateLna);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &LogEdit::highlightCurrentLine);

    bindInternalScroll();

    updateLnaWidth();

    QFont font;
    QSettings setting;
    font.setFamily(setting.value("editorFont").toString());
    font.setPointSize(setting.value("editorFontSize", 12).toInt());
    setFont(font);

    if (!setting.value("wrap").toBool())
        setLineWrapMode(QPlainTextEdit::NoWrap);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mHighlighter.reset(new Highlighter(document()));

    auto shortcut = new QShortcut(QKeySequence(QString("Ctrl+E")), this);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, [this]{
        QTextCursor cursor = textCursor();
        auto word = cursor.selectedText();
        if (word.isEmpty()) {
            QMessageBox::warning(this, "", "需要先选择一个单词再高亮");
            return;
        }

        mHighlighter->quickHighlight(word);
    });

    shortcut = new QShortcut(QKeySequence(QString("Ctrl+R")), this);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, [this]{
        QTextCursor cursor = textCursor();
        auto word = cursor.selectedText();
        if (word.isEmpty()) {
            QMessageBox::warning(this, "", "需要先选择一个单词再过滤");
            return;
        }

        emit filterRequested(word);
    });

    shortcut = new QShortcut(QKeySequence(QString("Ctrl+T")), this);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, [this]{
        QTextCursor cursor = textCursor();
        int lineNum = blockNumberToPresenter(cursor.blockNumber());
        emit addToTimeLineRequested(lineNum);
    });


}

void LogEdit::setMode(LogEdit::Mode mode)
{
    mMode = mode;
    if (mMode == WatchMode && mLog) {
        mExternalBar->setValue(mExternalBar->maximum());
    }
    emit onModeChanged(mode);
}

void LogEdit::onReplay(const Location &pos)
{
    scrollToLine(pos.line, pos.offset, false);
}

void LogEdit::keyPressEvent(QKeyEvent *e)
{
    mAltHold = e->modifiers().testFlag(Qt::AltModifier);
    QPlainTextEdit::keyPressEvent(e);
}

void LogEdit::keyReleaseEvent(QKeyEvent *e)
{
    mAltHold = e->modifiers().testFlag(Qt::AltModifier);
    QPlainTextEdit::keyReleaseEvent(e);
}

void LogEdit::setLog(std::shared_ptr<ILog> presenter)
{
    mHistory.clear();

    mLog = presenter;
    mExternalBar->setEnabled(false);

    if (mLog) {
        auto range = presenter->availRange();
        if (!range.isValid())//还没有数据
            return;

        updateExternalScrollRange(range);
        load(1);
    } else {
        clear();
    }
}

void LogEdit::setExternalVScrollbar(QScrollBar *bar)
{
    mExternalBar = bar;
    bindExternalScroll();
}

void LogEdit::onLogRangeChanged()
{
    //两种模式：跟随模式和回看模式
    //只有用户触发才会进入到跟随模式
    //一旦向上滚动，会自动进入到回看模式

    //回看模式下，如果还没填充，或填充没过半，就填充；如果填充足够，但是被clip，就clip，然后填充尾巴部分
    //跟随模式下，始终显示最新内容

    auto availRange = mLog->availRange();
    auto firstVisibleLineNumber = blockNumberToPresenter(firstVisibleBlock().blockNumber());

    if (mMode == LogEdit::ReviewMode) {
        if (!mBufferRange.isValid()) {
            load(1);
            return ;
        }

        //取buffer range和avail range交集，
        auto intersectRange = availRange.intersect(mBufferRange);

        if (intersectRange.isValid()) {
            //如果intersectRange等于buffer range，但buffer range不够大，也load(intersectRange.from)
            if (intersectRange == mBufferRange) {
                if (mBufferRange.to - firstVisibleLineNumber < 2 * mViewPortMaxLineCnt) {
                    load(firstVisibleLineNumber);
                }
            }
            //如果intersectRange不等于buffer range，则load(intersectRange.from)
            else {
                load(intersectRange.from, firstVisibleLineNumber);
            }
        } else {
            //如果完全没有交集，则load(avai range. from)
            firstVisibleLineNumber = availRange.from;
            load(availRange.from);
        }
    } else {
        auto availRange = mLog->availRange();
        auto to = availRange.to;
        auto from = to - 2 * mViewPortMaxLineCnt;
        if (from < availRange.from)
            from = availRange.from;

        mBufferRange.from = from;
        mBufferRange.to = to;

        unBindInternalScroll();

        auto horiValue = horizontalScrollBar()->value();
        setPlainText(mLog->readLines(from, to));
        horizontalScrollBar()->setValue(horiValue);

        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
        firstVisibleLineNumber = blockNumberToPresenter(firstVisibleBlock().blockNumber());

        bindInternalScroll();
    }

    unBindExternalScroll();
    updateExternalScrollRange(availRange);
    mExternalBar->setValue(firstVisibleLineNumber);
    bindExternalScroll();
}

void LogEdit::onLogEmphasized(int lineNum)
{
    scrollToLine(lineNum);
}

void LogEdit::reload()
{
    auto firstBlock = firstVisibleBlock().blockNumber();
    load(firstBlock+10, firstBlock);
}

//FIXME: qt 自作主张地会在最后加入一个空行：
//比如setPlainText("\n")，会得到：
//1.
//2.
void LogEdit::load(int preloadCenterLine, int firstVisibleLineNumber)
{
    auto availRange = mLog->availRange();

    auto from = preloadCenterLine - PRELOAD_COUNT/2;
    if (from < availRange.from)
        from = availRange.from;

    auto to = preloadCenterLine + PRELOAD_COUNT/2;
    if (to > availRange.to)
        to = availRange.to;

    mBufferRange.from = from;
    mBufferRange.to = to;

    unBindInternalScroll();

    auto horiValue = horizontalScrollBar()->value();
    setPlainText(mLog->readLines(from, to));
    horizontalScrollBar()->setValue(horiValue);
    if (firstVisibleLineNumber < 0)
        firstVisibleLineNumber = preloadCenterLine;
    auto firstBlockNum = presenterNumberToBlock(firstVisibleLineNumber);
    if (firstBlockNum < 0)
        firstBlockNum = 0;
    setVertialScrollbarByBlockNum(firstBlockNum);

    bindInternalScroll();
}

void LogEdit::setVertialScrollbarByBlockNum(int blockNumber)
{
    //scrollbar的范围是按wrap后的行数计算，所以这里不能直接传入blockNumber,而是取block.firstLineNumber
    verticalScrollBar()->setValue(document()->findBlockByNumber(blockNumber).firstLineNumber());
}

void LogEdit::handleInternalScroll(int)
{
    if (!mLog)
        return;

    if (mResizing)
        return;

    setMode(LogEdit::ReviewMode);

    auto firstVisibleLineNum = blockNumberToPresenter(firstVisibleBlock().blockNumber());
    unBindExternalScroll();
    mExternalBar->setValue(firstVisibleLineNum);
    bindExternalScroll();

    auto windowTop = firstVisibleLineNum-1;
    auto windowBottom = firstVisibleLineNum + mViewPortMaxLineCnt+1;
//    qDebug()<<"["<<windowTop<<","<<windowBottom<<"] vs "<<mBufferRange;
    if (windowTop < mBufferRange.from || windowBottom > mBufferRange.to) {
        load(firstVisibleLineNum);
    }
}

void LogEdit::handleExternalScroll(int value)
{
    auto blockNum = presenterNumberToBlock(value);
    if (blockNum == firstVisibleBlock().blockNumber())
        return;

    if (mBufferRange.contains(value)) {
        unBindInternalScroll();
        setVertialScrollbarByBlockNum(presenterNumberToBlock(value));
        bindInternalScroll();
    } else {
        load(value);
    }
}

void LogEdit::bindInternalScroll()
{
    if (mInternalVBarHandler)
        return;
    mInternalVBarHandler = connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &LogEdit::handleInternalScroll);
}

void LogEdit::unBindInternalScroll()
{
    if (mInternalVBarHandler)
        disconnect(mInternalVBarHandler);
}

void LogEdit::bindExternalScroll()
{
    if (mExternalVBarHandler)
        return;
    mExternalVBarHandler = connect(mExternalBar, &QScrollBar::valueChanged, this, &LogEdit::handleExternalScroll);
}

void LogEdit::unBindExternalScroll()
{
    if (mExternalVBarHandler)
        disconnect(mExternalVBarHandler);
}

void LogEdit::updateExternalScrollRange(Range range)
{
    auto min = range.from;
    auto max = range.to;// - mViewPortMaxLineCnt + 1;
    if (max < min)//数据还不够显示一屏
        max = min;

    mExternalBar->setRange(min, max);
    mExternalBar->setEnabled(true);
}

shared_ptr<LongtimeOperation> LogEdit::find(SearchArg arg, bool forward)
{
    auto cursor = textCursor();
    //清除选区以保证连续search的正确
    if (cursor.hasSelection()) {
        auto len = cursor.selectionEnd() - cursor.selectionStart();
        if (forward) {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, len);
        } else {
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, len);
        }
    }
    setTextCursor(cursor);

    Location l;
    l.line = blockNumberToPresenter(cursor.blockNumber());
    l.offset = cursor.positionInBlock();
    //因为text cursor的position指向后一个字符（比如对于竖线光标，position指示竖线后一个字符）
    //所以，为了避免反向搜索时搜索到自己，需要-1
    if (!forward && l.offset > 0) {
        l.offset -= 1;
    }

    return mLog->find(arg, l, forward);
}

void LogEdit::onLogFindOne(SearchResult ret)
{
    scrollToLine(ret.line, ret.offset);

    mHighlighter->clearSearchHighlight();

    //利用select选择search词
    auto cursor = textCursor();
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, ret.matchedText.length());
    setTextCursor(cursor);

    mHighlighter->searchHighlight(ret.matchedText, ret.arg.caseSensitive);
    repaint();
}

void LogEdit::scrollToLine(int lineNum, int col, bool recordToHistory)
{
    auto cursor = textCursor();
    if (recordToHistory)
        mHistory.push(blockNumberToPresenter(cursor.blockNumber()), cursor.columnNumber());

    handleExternalScroll(lineNum);//将target行滚动到窗口中间

    auto blockNum = presenterNumberToBlock(lineNum);
    auto block = document()->findBlockByNumber(blockNum);
    auto pos = block.position() + col;

    cursor.setPosition(pos);
    setTextCursor(cursor);

    if (recordToHistory)
        mHistory.push(lineNum, col);
    repaint();
}

static inline int calDigitCount(int d) {
    int cnt = 1;
    while (d >= 10) {
        d/=10;
        ++cnt;
    }
    return cnt;
}

int LogEdit::lnaWidth()
{
    auto maxLineDigitCnt = calDigitCount(mBufferRange.to);
    auto singleDigitWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
    ////左右各留2个像素
    return 4 + singleDigitWidth * maxLineDigitCnt;
}

void LogEdit::paintLna(QPaintEvent* event)
{
    QPainter painter(mLna);

    QColor bgColor(230,230,230);
    QColor fgColor(Qt::gray);
    if (mFocusManager->isInFocus(this)) {
        bgColor.setRgb(0, 153, 204);
        fgColor.setRgb(255, 255, 255);
    }

    painter.fillRect(event->rect(), bgColor);

    if (!mLog)
        return;

    QTextBlock block = firstVisibleBlock();
    auto lineNumber = blockNumberToPresenter(block.blockNumber());
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    auto curLine = blockNumberToPresenter(textCursor().blockNumber());

    painter.setPen(fgColor);
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            painter.save();
            if (lineNumber == curLine) {
                auto font = painter.font();
                font.setUnderline(true);
                font.setBold(true);
                painter.setFont(font);
            }
            painter.drawText(0, top, mLna->width(), fontMetrics().height(),
                             Qt::AlignRight, QString::number(lineNumber));
            painter.restore();
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++lineNumber;
    }
}

void LogEdit::updateLna(const QRect &rect, int dy)
{
    if (dy)
        mLna->scroll(0, dy);
    else
        mLna->update(0, rect.y(), mLna->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLnaWidth();
}

void LogEdit::updateLnaWidth(int)
{
    setViewportMargins(lnaWidth(), 0, 0, 0);
}

void LogEdit::resizeEvent(QResizeEvent *e)
{
    mResizing = true;
    QPlainTextEdit::resizeEvent(e);
    mResizing = false;

    auto metrics = fontMetrics();
    mViewPortMaxLineCnt = height()/metrics.height();

    QRect cr = contentsRect();
    mLna->setGeometry(QRect(cr.left(), cr.top(), lnaWidth(), cr.height()));
}

bool LogEdit::event(QEvent *e)
{
//    qDebug()<<e->type();
    return QPlainTextEdit::event(e);
}

void LogEdit::contextMenuEvent(QContextMenuEvent *e)
{
    QTextCursor cursor;

    if (mAltHold) {
        cursor = cursorForPosition(e->pos());
        cursor.select(QTextCursor::WordUnderCursor);
        setTextCursor(cursor);
    } else {
        cursor = textCursor();
    }

    QString cursorWord = cursor.selectedText();
    int lineNum = blockNumberToPresenter(cursor.blockNumber());

    emit menuRequested(e->globalPos(), cursorWord, lineNum);
}

int LogEdit::blockNumberToPresenter(int blockNumber)
{
    return blockNumber + mBufferRange.from;
}

int LogEdit::presenterNumberToBlock(int lineNumber)
{
    return lineNumber - mBufferRange.from;
}

void LogEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setTextOutline(QPen(Qt::black));
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selection.cursor.select(QTextCursor::BlockUnderCursor);
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

//FIXME: 用这种方式触发的，最终绕一圈回来调用scrollToLine会让目标行跑到第一行。如果目标行已经在视野内，就会很奇怪
void LogEdit::mouseDoubleClickEvent(QMouseEvent* e)
{
    QPlainTextEdit::mouseDoubleClickEvent(e);

    int lineNum = blockNumberToPresenter(textCursor().blockNumber());
    emit emphasizeLineRequested(lineNum);
}


