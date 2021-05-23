#include "documenttab.h"
#include <QSplitter>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <functional>
#include <QHBoxLayout>
#include "logviewer.h"
#include "progressdialog.h"
#include <QMessageBox>
#include <memory>
#include <util.h>
#include <QApplication>
#include "timeline.h"
#include "filterdialog.h"
#include "usercontrol.h"
#include <QMenu>
#include "sublog.h"
#include <QStack>
#include <QThread>
#include <QJsonArray>
#include <QInputDialog>
#include <QFileDialog>

DocumentTab::DocumentTab()
    : QWidget(nullptr)
{
    auto hsplitter = new QSplitter();

    mLogSplitter = new QSplitter();
    mLogSplitter->setOrientation(Qt::Vertical);

    connect(&mFocusManager, &FocusManager::focusChanged, [this](QWidget* old, QWidget* now){
        if (old) {
            old->repaint();
        }
        if (!now->hasFocus()) {
            now->setFocus();
        }

        loadTagListWithCurrentLogEdit(currentLogEdit());
    });

    mLogViewer = new LogViewer(&mFocusManager, true);
    mFocusManager.add(mLogViewer->display());
    mLogSplitter->addWidget(mLogViewer);

    mSubLogTabWidget = new QTabWidget();
    mSubLogTabWidget->setTabsClosable(true);
    mLogSplitter->addWidget(mSubLogTabWidget);
    mSubLogTabWidget->hide();
    connect(mSubLogTabWidget, &QTabWidget::tabCloseRequested, this, &DocumentTab::closeSubLog);

    mLogSplitter->setStretchFactor(0, 2);
    mLogSplitter->setStretchFactor(1, 1);

    hsplitter->addWidget(mLogSplitter);
    mTimeLine = new TimeLine;
    hsplitter->addWidget(mTimeLine);
    connect(mTimeLine, &TimeLine::nodeSelected, [this](TimeNode* node){
        auto rootLog = mLogViewer->display()->getLog();
        qApp->postEvent(this, new EmphasizeLineEvent(rootLog, node->lineNumber(), false));
    });

    hsplitter->setStretchFactor(0, 2);
    hsplitter->setStretchFactor(1, 1);

    auto box = new QHBoxLayout;
    box->addWidget(hsplitter);
    setLayout(box);
}

void DocumentTab::init(shared_ptr<ILogSource> log)
{
    mLogSource = log;
    mLogViewer->display()->setLog(mLogSource->getLog());
    connectLogEdit(mLogViewer->display());
}

void DocumentTab::connectLogEdit(LogEdit* edit)
{
    //taglist
    connect(edit->highlighter(), &Highlighter::onPatternAdded, [](HighlightPattern p){
        UserControl::instance().tagList()->addTag(p.key, p.color);
    });

    //emphasize
    //所有edit的emphasize请求都提交给root presenter
    connect(edit, &LogEdit::emphasizeLineRequested, [this, edit](int lineNum){
        qApp->postEvent(this, new EmphasizeLineEvent(edit->getLog(), lineNum, true));
    });

    //右键菜单
    connect(edit, &LogEdit::menuRequested,
            [this, edit](QPoint point, const QString& cursorWord, int lineNum){
        auto menu = new QMenu;
        auto a = menu->addAction("添加当前行到时间线");
        connect(a, &QAction::triggered, [this, edit, lineNum]{
            auto log = edit->getLog();

            auto sourceLine = log->toRootLine(lineNum);
            auto text = log->readLine(sourceLine);
            mTimeLine->addNode(sourceLine, text);
        });

        if (!cursorWord.isEmpty()) {
            menu->addSeparator();

            auto showWord=cursorWord;
            if (cursorWord.length() > 15)
                showWord = cursorWord.mid(0, 12)+"...";

            a = menu->addAction("高亮"+showWord);
            connect(a, &QAction::triggered, [edit, cursorWord]{
                edit->highlighter()->quickHighlight(cursorWord);
            });

            a = menu->addAction("搜索"+showWord);
            connect(a, &QAction::triggered, [cursorWord]{
                auto searchBar = UserControl::instance().searchBar();
                searchBar->fill(cursorWord);
                searchBar->buildSearchRequest();
            });

            a = menu->addAction("过滤"+showWord);
            connect(a, &QAction::triggered, [this, cursorWord]{
                SearchArg arg;
                arg.pattern = cursorWord;
                arg.caseSensitive = true;
                doFilter(arg);
            });

            a = menu->addAction("反转过滤"+showWord);
            connect(a, &QAction::triggered, [this, cursorWord]{
                SearchArg arg;
                arg.pattern = cursorWord;
                arg.revert = true;
                doFilter(arg);
            });
        }

        a = menu->addAction("清除搜索高亮");//更好的交互方式
        connect(a, &QAction::triggered, edit->highlighter(), &Highlighter::clearSearchHighlight);

        menu->addSeparator();
        a = menu->addAction("复制");
        connect(a, &QAction::triggered, edit, &LogEdit::copy);

        menu->exec(point);
    }
    );
}

void DocumentTab::modalLongOp(shared_ptr<LongtimeOperation> op, QString hint)
{
    mLogSource->pause();

    QEventLoop loop;//等待队列中可能存在的LogChange事件被处理
    loop.processEvents(QEventLoop::ExcludeUserInputEvents);

    if (!op->isCanceled())
        new ProgressDialog(op, hint, [this]{mLogSource->resume();});
}

void DocumentTab::onSubLogCreated(shared_ptr<SubLog> subLog)
{
    if (!subLog->availRange().isValid()) {
        QMessageBox::warning(this, "过滤结果", "没有过滤到任何匹配行");
        return;
    }

    auto viewer = new LogViewer(&mFocusManager, true);

    auto subEdit = viewer->display();
    subEdit->setLog(subLog);

    connectLogEdit(subEdit);
    appendSubLog(viewer, subLog->getSearchArg().pattern);
}

void DocumentTab::onFindDone(shared_ptr<ILog> who, SearchResult ret)
{
    if (ret.matchedText.isEmpty()) {
        if (ret.line <= 1) {
            QMessageBox::warning(this, "查找", "查找到顶部，没有找到匹配结果");
        } else {
            QMessageBox::warning(this, "查找", "查找到底部，没有找到匹配结果");
        }
        return;
    }
    auto edit = findLogEditByLog(who);
    if (edit) {
        edit->onLogFindOne(ret);
    }
}

void DocumentTab::onLogChange(shared_ptr<ILog> who, EventAction update)
{
    auto before = who->availRange();
    update();
    auto after = who->availRange();

    auto edit = findLogEditByLog(who);
    if (edit)
        edit->onLogRangeChanged();

    for (auto&& child: who->children()) {
        deepHandleLogChange(child, before, after);
    }
}

void DocumentTab::onLineEmphasized(shared_ptr<ILog> who, int line, bool isSource)
{
    auto rootLine = who->toRootLine(line);
    auto root = who->rootLog();

    //从上到下，触发emphasize
    deepEmphasizeLine(root, rootLine, isSource ? who : shared_ptr<ILog>());
}

void DocumentTab::deepHandleLogChange(const shared_ptr<ILog> &log, Range before, Range after)
{
    auto myBefore = log->availRange();
    if (!log->onParentRangeChanged(before, after))
        return;
    auto myAfter = log->availRange();

    auto edit = findLogEditByLog(log);
    if (edit) {
        edit->onLogRangeChanged();
    }

    for (auto&& child : log->children()) {
        deepHandleLogChange(child, myBefore, myAfter);
    }
}

void DocumentTab::deepEmphasizeLine(const shared_ptr<ILog> &log, int line, const shared_ptr<ILog> &source)
{
    //高亮自己
    if (log != source) {//只在非源的log上刷新。否则会出现双击某个文本框时，自己这个文本框的高亮行跳到了第一行位置
        auto edit = findLogEditByLog(log);
        if (edit)
            edit->onLogEmphasized(line);
    }

    //高亮子节点
    auto children = log->children();
    for (auto&& child : children) {
        deepEmphasizeLine(child, child->fromParentLine(line), source);
    }
}

void DocumentTab::unInit()
{
    if (mLogSource)
        mLogSource->close();
}

#define CONNECT(intent, functor) \
{\
    auto a = UserControl::instance().actionFor(intent);\
    mConnections.push_back(connect(a, &QAction::triggered, functor));\
}

#define CONNECT1(intent, who, functor) \
{\
    auto a = UserControl::instance().actionFor(intent);\
    mConnections.push_back(connect(a, &QAction::triggered, who, functor));\
}

void DocumentTab::connectUserControls()
{
    auto actions = UserControl::instance();

    mConnections.push_back(connect(actions.searchBar(),
            &SearchBar::searchRequest,
            this,
            &DocumentTab::find));

    CONNECT(UserControl::GoBackwardIntent, [this]{currentLogEdit()->history()->backward();});
    CONNECT(UserControl::GoForwardIntent, [this]{currentLogEdit()->history()->forward();});
    CONNECT1(UserControl::FilterIntent, this, &DocumentTab::filter);
    CONNECT1(UserControl::LocateLineIntent, this, &DocumentTab::locateLine);
    CONNECT1(UserControl::HighlightIntent, this, &DocumentTab::addHighlight);
    CONNECT1(UserControl::FindPreviousIntent, actions.searchBar(), &SearchBar::backwardBtnClicked);
    CONNECT1(UserControl::FindNextIntent, actions.searchBar(), &SearchBar::forwardBtnClicked);
    CONNECT(UserControl::FindIntent, []{
        auto searchBar = UserControl::instance().searchBar();
        searchBar->setSearchForward(true);
        searchBar->strongFocus();
    });
    CONNECT(UserControl::RevertFindIntent, []{
        auto searchBar = UserControl::instance().searchBar();
        searchBar->setSearchForward(false);
        searchBar->strongFocus();
    });
    CONNECT1(UserControl::CopyTimeLineIntent, mTimeLine, &TimeLine::exportToClipboard);
    CONNECT1(UserControl::SaveTimeLineIntent, this, &DocumentTab::exportTimeLineToImage);
    CONNECT1(UserControl::ClearTimeLineIntent, mTimeLine, &TimeLine::clear);


    //taglist
    auto taglist = UserControl::instance().tagList();
    mConnections.push_back(connect(taglist, &TagListWidget::onTagDeleted, [this](const QString& text){
        currentLogEdit()->highlighter()->clearQuickHighlight(text);
    }));
    mConnections.push_back(connect(taglist, &TagListWidget::requestSearchTag, [](const QString& keyword){
        auto searchBar = UserControl::instance().searchBar();
        searchBar->fill(keyword);
        searchBar->buildSearchRequest();
    }));
    mConnections.push_back(connect(taglist, &TagListWidget::onTagColorChanged, [this](const QString& keyword, QColor color){
        currentLogEdit()->highlighter()->quickHighlight(keyword, color);
    }));
    mConnections.push_back(connect(taglist, &TagListWidget::requestFilterTag, this, &DocumentTab::doFilter));

    loadTagListWithCurrentLogEdit(currentLogEdit());
}

void DocumentTab::loadTagListWithCurrentLogEdit(LogEdit* edit)
{
    auto taglist = UserControl::instance().tagList();
    taglist->clear();

    for (auto& p : edit->highlighter()->allQuickHighlights()) {
        taglist->addTag(p.key, p.color);
    }
}

void DocumentTab::disconnectUserControls()
{
    for (auto& conn : mConnections)
        disconnect(conn);
    mConnections.clear();
}

QJsonValue DocumentTab::saveToJson()
{
    QJsonObject o;
    o["source"] = mLogSource->saveToJson();
    o["timeline"] = mTimeLine->saveToJson();
    o["name"] = mName;

    auto root = mLogSource->getLog();
    QJsonArray logs;
    deepSaveLogToJson(root, logs);

    o["logs"] = logs;

    return o;
}

void DocumentTab::deepSaveLogToJson(shared_ptr<ILog> &log, QJsonArray &o)
{
    QJsonObject me;
    me["type"] = log->getType();
    log->setId(QString("%1").arg((quint64)log.get()));
    me["id"] = log->getId();
    if (log->parent()) {
        me["parent"] = log->parent()->getId();
    }

    if (log->getType() == "sub") {
        me["searchArg"] = ((SubLog*)log.get())->getSearchArg().saveToJson();
    }

    QJsonArray jsonChildren;
    for (auto&& child : log->children()) {
        deepSaveLogToJson(child, jsonChildren);
    }
    me["children"]=jsonChildren;

    o.append(me);
}

bool DocumentTab::loadFromJson(const QJsonValue &o)
{
    mLoadingPharse.reset(new LoadingPhase);

    shared_ptr<ILogSource> source(ILogSource::createLogSourceFromType(o["source"].toObject()["type"].toString(), this));
    if (source) {
        source->loadFromJson(o["source"]);
        init(source);
        if (!source->open()) {
            QMessageBox::critical(this, "无法打开文件", "文件打开失败，跳过该文件");
            return false;
        }

        auto jrootLog = o["logs"].toArray()[0].toObject();
        auto rootLog = source->getLog();
        rootLog->setId(jrootLog["id"].toString());
        for (auto child : jrootLog["children"].toArray()) {
            deepCreateSubLogFromJson(child.toObject());
        }
    }

    mTimeLine->loadFromJson(o["timeline"]);
    mName = o["name"].toString();

    return true;
}

void DocumentTab::deepCreateSubLogFromJson(QJsonObject o)
{
    if (o["type"]=="sub") {
        //todo 给sublog设置id
        auto action = [this, o]{
            qDebug()<<"create sub log: "<<o;
            auto parentLog = mLogSource->getLog()->findLogById(o["parent"].toString());
            SearchArg arg;
            arg.loadFromJson(o["searchArg"].toObject());
            if (parentLog) {
                auto op = parentLog->createChild(arg, o["id"].toString());
                modalLongOp(op, "过滤中……可能需要较长时间");
            }
        };
        mLoadingPharse->addAction(action);
    }

    for (auto child : o["children"].toArray()) {
        deepCreateSubLogFromJson(child.toObject());
    }
}

void DocumentTab::find(SearchArg arg, bool forward)
{
    auto* logEdit = currentLogEdit();

    auto op = logEdit->find(arg, forward);

    modalLongOp(op, "查找中……可能需要较长时间");
}

void DocumentTab::filter()
{
    //如果这个地方的逻辑放行，filter的时候以为需要暂停住log change，
    //这导致filter结束的时候会有大量的on log change 处理，而sub log的同步逻辑较为低效（且在主线程执行）
    //这就导致主线程被卡死
    if (!mLogSource->getLoadProgress()->isDone()) {
        QMessageBox::warning(this, "过滤", "日志还未完全加载完成，请耐心等待……");
        return;
    }

    FilterDialog dlg;
    if (dlg.exec() == QDialog::Rejected)
        return;

    doFilter(dlg.getArg());
}

void DocumentTab::setCodec(QString name)
{
    mLogSource->setCodec(name);
    mLogViewer->display()->reload();

    auto cnt = mSubLogTabWidget->count();
    for (auto i = 0; i < cnt; i++) {
        auto viewer = (LogViewer*)mSubLogTabWidget->widget(i);
        viewer->display()->reload();
    }
}

void DocumentTab::customEvent(QEvent *ev)
{
    auto type = (EventsType)ev->type();
    switch (type) {
    case evLogChange: {
        auto e = (LogChangeEvent*)ev;
        onLogChange(e->who, e->updateLog);
        ev->accept();
        break;
    }
    case evSubLogCreated:{
        auto e = (SubLogCreatedEvent*)ev;
        onSubLogCreated(e->log);
        ev->accept();
        break;
    }
    case evEmphasizeLine:{
        auto e = (EmphasizeLineEvent*)ev;
        onLineEmphasized(e->who, e->line, e->isSource);
        ev->accept();
        break;
    }
    case evFindDone:{
        auto e = (FindDoneEvent*)ev;
        onFindDone(e->who, e->ret);
        ev->accept();
        break;
    }
    case evError:{
        auto e = (ErrorEvent*)ev;
        e->cleanup();
        ev->accept();
        break;
    }
    case evSourceReady: {
        ev->accept();
        break;
    }
    default:
        ev->ignore();
        break;
    }

    if (mLoadingPharse) {
        if (!mLoadingPharse->process(ev)) {
            mLoadingPharse.reset();
        }
    }
}

void DocumentTab::locateLine()
{
    auto edit = currentLogEdit();
    auto log = edit->getLog();
    auto range = log->availRange();

    bool ok = false;
    auto lineNum = QInputDialog::getInt(this,
            "跳转到行",
            QString("范围：%1 - %2").arg(range.from).arg(range.to),
            range.from,
            range.from, range.to, 1,
            &ok);

    if (ok) {
        edit->scrollToLine(lineNum);
    }
}

void DocumentTab::addHighlight()
{
    auto text = QInputDialog::getText(this, "高亮", "输入要高亮的单词(大小写敏感)");
    if (text.length() > 0) {
        currentLogEdit()->highlighter()->quickHighlight(text);
    }
}

void DocumentTab::exportTimeLineToImage()
{
    auto savepath = QFileDialog::getSaveFileName(this, "导出时间线", QString(), "*.png");
    if (savepath.isEmpty())
        return;
    if (!savepath.endsWith(".png"))
        savepath+=".png";

    mTimeLine->exportToImage(savepath);
}

void DocumentTab::doFilter(SearchArg arg)
{
    auto log = currentLogEdit()->getLog();
    auto op = log->createChild(arg);

    modalLongOp(op, "过滤中……可能需要较长时间");
}

void DocumentTab::appendSubLog(LogViewer *logViewer, const QString &hint)
{
    mFocusManager.add(logViewer->display());
    mSubLogTabWidget->addTab(logViewer, hint);
    mSubLogTabWidget->setCurrentWidget(logViewer);
    if (mSubLogTabWidget->count() == 1) {
        mSubLogTabWidget->show();
    }
}

void DocumentTab::closeSubLog(int index)
{
    auto subViewer = (LogViewer*)mSubLogTabWidget->widget(index);
    mFocusManager.remove(subViewer->display());
    mSubLogTabWidget->removeTab(index);
    delete subViewer;

    if (mSubLogTabWidget->count() == 0) {
        mSubLogTabWidget->hide();
    }
}

LogEdit *DocumentTab::currentLogEdit()
{
    return (LogEdit*)mFocusManager.current();
}

LogEdit *DocumentTab::findLogEditByLog(const shared_ptr<ILog> &log)
{
    if (mLogViewer->display()->getLog() == log) {
        return mLogViewer->display();
    }

    auto cnt = mSubLogTabWidget->count();
    for (auto i = 0; i < cnt; i++) {
        auto viewer = (LogViewer*)mSubLogTabWidget->widget(i);
        if (viewer->display()->getLog() == log) {
            return viewer->display();
        }
    }

    return nullptr;
}


bool LoadingPhase::process(QEvent *ev)
{
    auto type = (EventsType)ev->type();
    qDebug()<<mState<<type;
    switch (mState) {
    case eWaitSourceReady:{
        if (type == evSourceReady) {
            mState = eProcessPendings;
            return runFirstAction();
        }
        break;
    }
    case eProcessPendings: {
        if (type == evSubLogCreated) {
            return runFirstAction();
        }
        break;
    }
    default:
        break;
    }

    return true;
}

void LoadingPhase::addAction(const function<void ()> &action)
{
    mPendingActions.push_back(action);
}

bool LoadingPhase::runFirstAction()
{
    if (mPendingActions.empty()) {
        mState = eFinish;
        return false;
    }

    auto action = mPendingActions.front();
    action();
    mPendingActions.pop_front();

    return true;
}
