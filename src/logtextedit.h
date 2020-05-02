#ifndef LOGTEXTEDIT_H
#define LOGTEXTEDIT_H

#include <QPlainTextEdit>
#include <QVector>
#include <QFont>
#include <QSyntaxHighlighter>
#include <QSet>
#include "log.h"
#include "history.h"

class Highlighter;
class LineNumberArea;
class LogTextEdit: public QPlainTextEdit, public AbstractNavPosReplayer
{
    Q_OBJECT

public:
    LogTextEdit(QWidget* parent);
    ~LogTextEdit() override;

//===实现大文件的显示===>
public:
    void setLog(Log* log);
    Log* getLog(){return mLog;}
    void setScrollBar(QScrollBar* scrollbar);
protected:
    void resizeEvent(QResizeEvent *e) override;
private:
    void load(int topLine);
    int fromViewPortToLog(int index);
    int fromLogToViewPort(int line);
    int centerLineGetTopRow(int line/*or index*/);
private slots:
    void handleInternalScroll(int value);
    void handleExternalScroll(int value);
private:
    Log* mLog{nullptr};
    int mFromLine{0};
    int mToLine{0};
    int mPreloadLineCnt{200};//预加载多少行
    bool mLoading{false};
    bool mScrolling{false};
    int mViewPortLineCnt{0};
    QScrollBar* mExternalBar;
//<===实现大文件的显示===

//===实现行号===>
private:
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
public:
    int getLineCount(){return mLog->lineCount();}
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);
private:
    friend class LineNumberArea;
    LineNumberArea *mLineNumArea;
//<===实现行号===

//===实现高亮===>
private slots:
    void highlightCurrentLine();
    void addWordHighlightUnderCursor();
private:
    Highlighter* mHighlighter;
//<===实现高亮===

//===实现搜索===>
public:
    bool search(const QString& text, QTextDocument::FindFlags options);
//<===实现搜索===

//===实现时间线===>
signals:
    void requestMarkLine(int lineNum, const QString& s);

private slots:
    void handleMarkLineAction();
//<===实现时间线===

//===实现导航历史==>
public:
    History* history(){return &mHistory;}
    void onReplay(const NavPos& pos) override;
private:
    History mHistory{this};
//<===实现导航历史==

//===实现主从窗口焦点切换==>
signals:
    void beenFocused(LogTextEdit*);
    void requestLocateMaster(int lineNum);
public:
    void setSlave(bool value);
    void drawFocused();
    void drawUnFocused();
protected:
    void focusInEvent(QFocusEvent *e) override;
private:
    bool mFocused{false};
    bool mIsSlave{false};
//<===实现主从窗口焦点切换==

//===其他===>
protected:
    void contextMenuEvent(QContextMenuEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
public:
    void setVisible(bool visible) override;
public:
    void scrollToLine(int lineNum, int col = 0, bool recordToHistory=true);
//<===其他===

};

#endif // LOGTEXTEDIT_H
