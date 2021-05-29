#ifndef LOGEDIT_H
#define LOGEDIT_H

#include <QPlainTextEdit>
#include "focusmanager.h"
#include "util.h"
#include "history.h"
#include "highlighter.h"
#include "ilog.h"
#include <QShortcut>

class LineNumberArea;
class LogEdit : public QPlainTextEdit, public AbstractNavPosReplayer
{
    Q_OBJECT
public:
    enum Mode{
        ReviewMode,
        WatchMode
    };
    LogEdit(FocusManager* focusManager);
    shared_ptr<ILog> getLog(){return mLog;}

public:
    void setMode(Mode mode);
signals:
    void onModeChanged(Mode newMode);

//===实现导航历史==>
public:
    History* history(){return &mHistory;}
    void onReplay(const Location &pos) override;
private:
    History mHistory{this};
//<===实现导航历史==

//菜单
protected:
    void contextMenuEvent(QContextMenuEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
signals:
    void menuRequested(QPoint point, const QString& cursorWord, int lineNum);
private:
    bool mAltHold{false};

//log对接
public:
    void setLog(std::shared_ptr<ILog> log);
    void setExternalVScrollbar(QScrollBar* bar);

public:
    void onLogRangeChanged();
    void onLogEmphasized(int lineNum);
    void reload();

private:
    void load(int preloadCenterLine, int firstVisibleLineNumber=-1);
    void setVertialScrollbarByBlockNum(int blockNumber);
    void handleInternalScroll(int);
    void handleExternalScroll(int value);
    void bindInternalScroll();
    void unBindInternalScroll();
    void bindExternalScroll();
    void unBindExternalScroll();
    void updateExternalScrollRange(Range range);

//搜索
public:
    shared_ptr<LongtimeOperation> find(SearchArg arg, bool forward);
    void onLogFindOne(SearchResult ret);

signals:
    void filterRequested(QString word);

//行号
private:
    friend class LineNumberArea;
    int lnaWidth();
    void paintLna(QPaintEvent* event);
    void updateLna(const QRect &rect, int dy);
    void updateLnaWidth(int newBlockCount = 0);

protected:
    void resizeEvent(QResizeEvent *e) override;
    bool event(QEvent* e) override;


private:
    int blockNumberToPresenter(int blockNumber);
    int presenterNumberToBlock(int lineNumber);

//高亮
public:
    void scrollToLine(int lineNum, int col=0, bool recordToHistory=true);
    Highlighter* highlighter() {return mHighlighter.get();}
protected:
    void mouseDoubleClickEvent(QMouseEvent* e) override;
signals:
    void emphasizeLineRequested(int lineNum);
    void addToTimeLineRequested(int lineNum);
private:
    void highlightCurrentLine();

private:
    QMetaObject::Connection mInternalVBarHandler;
    bool mResizing{false};
    QMetaObject::Connection mExternalVBarHandler;

    Range mBufferRange;
    LineNumberArea* mLna{nullptr};
    std::shared_ptr<ILog> mLog{nullptr};
    QScrollBar* mExternalBar{nullptr};
    int mViewPortMaxLineCnt{0};
    FocusManager* mFocusManager;

    Mode mMode{LogEdit::ReviewMode};

    std::unique_ptr<Highlighter> mHighlighter;
};

class LineNumberArea : public QWidget//lna => LineNumberArea
{
public:
    LineNumberArea(LogEdit *editor) : QWidget(editor), logEditor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(logEditor->lnaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        logEditor->paintLna(event);
    }

private:
    LogEdit *logEditor;
};

#endif // LOGEDIT_H
