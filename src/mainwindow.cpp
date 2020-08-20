#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDebug>
#include <QScrollBar>
#include "timeline.h"
#include "timenode.h"
#include <QTime>
#include <QToolBar>
#include <QStyle>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "history.h"
#include <QSplitterHandle>
#include "backgroundrunner.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QResizeEvent>
#include "toast.h"
#include <QMenu>
#include <QTextCodec>
#include <QDesktopServices>
#include "settingsdialog.h"
#include <QSettings>
#include <QToolButton>
#include <QCheckBox>
#include <QListWidget>
#include "searchedit.h"
#include "taglistwidget.h"
#include <QMimeData>
#include <QDockWidget>
#include "updater.h"
#include "aboutdlg.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("loginsight");
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(&Updater::instance(), &Updater::newVersionFound, [](const QString& v){
        QMessageBox mbox;
        mbox.setTextFormat(Qt::RichText);
        mbox.setWindowTitle("软件更新");
        mbox.setText("<p>找到新版本: "+v+"</p><a href='https://github.com/compilelife/loginsight/releases/latest'>前往下载</a>");
        mbox.exec();
    });
    Updater::instance().checkNewVersion();

    createCenterWidget();
    createSubLogDockWidget();
    createTimelineDockWidget();
    createToolbar();
    createTagbar();
    bindActions();

    showMaximized();

    QSettings config;
    mCaseSensitiveCheckBox->setChecked(config.value("caseSensitive").toBool());

    noDocSetDisable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bindActions()
{
    connect(mSearchEdit, SIGNAL(searchFoward()), this, SLOT(handleSearchFoward()));
    connect(mSearchEdit, SIGNAL(searchBackward()), this, SLOT(handleSearchBackward()));

    connect(mTimeLine, SIGNAL(nodeSelected(TimeNode*)), this, SLOT(handleNodeSelected(TimeNode*)));

    bindLogEditActions();
    bindMenuAction();
}

void MainWindow::bindLogEditActions()
{
    connect(mLogEdit, SIGNAL(requestMarkLine(int, const QString&)), mTimeLine, SLOT(addNode(int,const QString&)));
    connect(mSubLogEdit, SIGNAL(requestMarkLine(int, const QString&)), this, SLOT(handleSubLogMarkLine(int, const QString&)));
    connect(mLogEdit->history(), SIGNAL(posChanged()), this, SLOT(handleHistoryPosChanged()));
    connect(mSubLogEdit->history(), SIGNAL(posChanged()), this, SLOT(handleHistoryPosChanged()));
    connect(mLogEdit, SIGNAL(returnPressed()), mSearchEdit, SLOT(transferReturnBehavior()));
    connect(mSubLogEdit, SIGNAL(returnPressed()), mSearchEdit, SLOT(transferReturnBehavior()));
    connect(mLogEdit, &LogTextEdit::beenFocused, this, &MainWindow::handleLogEditFocus);
    connect(mSubLogEdit, &LogTextEdit::beenFocused, this, &MainWindow::handleLogEditFocus);
    connect(mLogEdit, SIGNAL(emphasizeLine(int)), this, SLOT(handleLogEditEmphasizeLine(int)));
    connect(mSubLogEdit, SIGNAL(emphasizeLine(int)), this, SLOT(handleSubLogEditEmphasizeLine(int)));
    connect(mLogEdit, SIGNAL(requestFilter(const QString&)), this, SLOT(handleFilterRequest(const QString&)));
    connect(mSubLogEdit, SIGNAL(requestFilter(const QString&)), this, SLOT(handleFilterRequest(const QString&)));
    auto searchUsingCurDirection = [this](const QString& text){
        mSearchEdit->setText(text);
        search(mSearchEdit->isSearchFoward());
    };
    connect(mLogEdit, &LogTextEdit::delegateSearch, searchUsingCurDirection);
    connect(mSubLogEdit, &LogTextEdit::delegateSearch, searchUsingCurDirection);

    connect(mLogEdit, &LogTextEdit::userDropFile, [this](const QString& path){
        doOpenFile(path);
    });
}

void MainWindow::bindMenuAction()
{
    //文件
    auto group = new QActionGroup(this);
    group->addAction(ui->actionGBK);
    group->addAction(ui->actionUTF8);

    connect(group, &QActionGroup::triggered, [this](QAction* action){setEncoding(action->text());});
    connect(ui->actionopen, &QAction::triggered, this, &MainWindow::handleOpenFile);
    connect(ui->actionclose, &QAction::triggered, this, &MainWindow::handleCloseFile);
    connect(ui->actionquit, &QAction::triggered, []{QCoreApplication::quit();});
    connect(ui->actionsetting, &QAction::triggered, []{
        SettingsDialog dlg;
        dlg.exec();
    });

    //检视
    connect(ui->actionsearch, &QAction::triggered, [this]{
        mSearchEdit->setSearchFoward(true);
        mSearchEdit->setFocus();
    });
    connect(ui->actionreverseSearch, &QAction::triggered, [this]{
        mSearchEdit->setSearchFoward(false);
        mSearchEdit->setFocus();
    });
    connect(ui->actionsearchFoward, &QAction::triggered, this, &MainWindow::handleSearchFoward);
    connect(ui->actionsearchBack, &QAction::triggered, this, &MainWindow::handleSearchFoward);
    connect(ui->actionfilter, &QAction::triggered, this, &MainWindow::handleFilter);
    connect(ui->actiongoBack, &QAction::triggered, this, &MainWindow::handleNavBackward);
    connect(ui->actiongoFoward, &QAction::triggered, this, &MainWindow::handleNavFoward);
    connect(ui->actiongotoLine, &QAction::triggered, this, &MainWindow::handleGotoLine);
    connect(ui->actionhighlight, &QAction::triggered, [this]{
        auto text = QInputDialog::getText(this, "高亮", "输入要高亮的单词(大小写敏感)");
        if (text.length() > 0) {
            mCurLogEdit->getHighlighter()->quickHighlight(text);
        }
    });

    //时间线
    connect(ui->actiontoClipboard, &QAction::triggered, mTimeLine, &TimeLine::exportToClipboard);
    connect(ui->actionexport, &QAction::triggered, this, &MainWindow::handleExportTimeLine);
    connect(ui->actionclearAll, &QAction::triggered, mTimeLine, &TimeLine::clear);

    //帮助
    connect(ui->actionshortcut, &QAction::triggered, [this]{
        mShortcutHelpDlg.show();
    });
    connect(ui->actiondoc, &QAction::triggered, []{
        QDesktopServices::openUrl(QUrl("https://github.com/compilelife/loginsight/wiki"));
    });
    connect(ui->actionabout, &QAction::triggered, []{
        AboutDlg dlg;
        dlg.exec();
    });
}

void MainWindow::noDocSetDisable()
{
    ui->actionclose->setEnabled(false);
    ui->menuEncoding->setEnabled(false);
    ui->menuInsight->setEnabled(false);

    ui->actiongoBack->setEnabled(false);
    ui->actiongoFoward->setEnabled(false);
    mNavBackAction->setEnabled(false);
    mNavAheadAction->setEnabled(false);

    mSearchEdit->setEnabled(false);
    mGotoLineAction->setEnabled(false);
    mFilterAction->setEnabled(false);

    mTagList->clear();
}

void MainWindow::hasDocSetEnbale()
{
    ui->actionclose->setEnabled(true);
    ui->menuEncoding->setEnabled(true);
    ui->menuInsight->setEnabled(true);

    ui->actiongoBack->setEnabled(false);
    ui->actiongoFoward->setEnabled(false);
    mNavBackAction->setEnabled(false);
    mNavAheadAction->setEnabled(false);

    mSearchEdit->setEnabled(true);
    mGotoLineAction->setEnabled(true);
    mFilterAction->setEnabled(true);

    mTagList->clear();
}

void MainWindow::handleExportTimeLine()
{
    auto filename = QFileDialog::getSaveFileName(this, "保存时间线到", QString(), "Images (*.png)");
    if (!filename.contains('.')) {
        filename += ".png";
    }
    mTimeLine->exportToImage(filename);
}

void MainWindow::handleFilter()
{
    if (!mLog.isOpened())
        return;

    QDialog inputDlg;
    inputDlg.setWindowTitle("过滤日志");

    QVBoxLayout layout(&inputDlg);
    QCheckBox caseSensentiveCheckBox("大小写敏感");
    QSettings config;
    caseSensentiveCheckBox.setChecked(config.value("caseSensitive").toBool());
    caseSensentiveCheckBox.setFocusPolicy(Qt::NoFocus);
    layout.addWidget(&caseSensentiveCheckBox);

    QLineEdit lineEdit;
    lineEdit.setPlaceholderText("请输入要过滤的关键字");
    lineEdit.setFocus();
    layout.addWidget(&lineEdit);

    QDialogButtonBox btnBox(QDialogButtonBox::Ok);
    layout.addWidget(&btnBox);
    connect(&btnBox, SIGNAL(accepted()), &inputDlg, SLOT(accept()));

    if (inputDlg.exec() != QDialog::Accepted || lineEdit.text().isEmpty()) {
        return;
    }

    auto text = lineEdit.text();
    filter(text, caseSensentiveCheckBox.isChecked());
}

void MainWindow::handleFilterRequest(const QString& text)
{
    QSettings config;
    filter(text, config.value("caseSensitive").toBool());
}

void MainWindow::handleNodeSelected(TimeNode *node)
{
    auto lineNum = node->data();
    mLogEdit->scrollToLine(lineNum);
    if (mSubLog) {
        auto subLogLine = mSubLog->fromParentLine(lineNum);
        if (subLogLine > 0)
            mSubLogEdit->scrollToLine(subLogLine);
    }
}

void MainWindow::handleSearchBackward()
{
    search(false);
}

void MainWindow::handleSearchFoward()
{
    search(true);
}

void MainWindow::handleGotoLine()
{
    if (!mLog.isOpened())
        return;

    bool ok = false;
    auto log = mCurLogEdit->getLog();

    auto lineNum = QInputDialog::getInt(this,
                                        "跳转到行",
                                        QString("范围：1 - %1").arg(log->lineCount()),
                                        1, 1, mCurLogEdit->getLineCount(),1,&ok);
    if (ok)
        mCurLogEdit->scrollToLine(lineNum);
}

void MainWindow::handleOpenFile()
{
    auto path = QFileDialog::getOpenFileName(this, "打开日志文件");
    if (path.isEmpty()) {
        return;
    }

    doOpenFile(path);
}

void MainWindow::handleCloseFile()
{
    if (mSubLog) {
        delete mSubLog;
        mSubLog = nullptr;
        mSubLogEdit->clear();
        mSubLogDWidget->setVisible(false);
    }

    mLog.close();
    mLogEdit->clear();
    mTimeLine->clear();

    noDocSetDisable();

    setWindowTitle("loginsight");
}

void MainWindow::handleHistoryPosChanged()
{
    mNavBackAction->setEnabled(mCurLogEdit->history()->availableBackwardCount() > 0);
    ui->actiongoBack->setEnabled(mCurLogEdit->history()->availableBackwardCount() > 0);
    mNavAheadAction->setEnabled(mCurLogEdit->history()->availableFowardCount() > 0);
    ui->actiongoFoward->setEnabled(mCurLogEdit->history()->availableFowardCount() > 0);
}

void MainWindow::handleNavBackward()
{
    mCurLogEdit->history()->backward();
}

void MainWindow::handleNavFoward()
{
    mCurLogEdit->history()->foward();
}

void MainWindow::handleLogEditFocus(LogTextEdit *logEdit)
{
    if (logEdit != mCurLogEdit) {
        mCurLogEdit->drawUnFocused();
        logEdit->drawFocused();
        mCurLogEdit = logEdit;

        handleHistoryPosChanged();

        mTagList->clear();
        auto highlighter = mCurLogEdit->getHighlighter();
        for (auto& p : highlighter->allQuickHighlights()) {
            mTagList->addTag(p.key, p.color);
        }

        if (mAddTagConnection) {
            disconnect(mAddTagConnection);
        }

        mAddTagConnection = connect(highlighter, &Highlighter::onPatternAdded, [this](HighlightPattern p){
            mTagList->addTag(p.key, p.color);
        });
    }
}

void MainWindow::setEncoding(const QString& text)
{
    if (text != mLog.getCodec()->name()) {
        qDebug()<<"change to encoding:"<<text;
        mLog.setCodec(QTextCodec::codecForName(text.toStdString().c_str()));
        mLogEdit->refresh();
        if (mSubLogEdit->isVisible()) {
            mSubLogEdit->refresh();
        }
    }
}

void MainWindow::handleLogEditEmphasizeLine(int lineNum)
{
    if (mSubLog) {
        auto subLogLine = mSubLog->fromParentLine(lineNum);
        if (subLogLine > 0)
            mSubLogEdit->scrollToLine(subLogLine);
    }

    mTimeLine->highlightNode(lineNum);
}

void MainWindow::handleSubLogEditEmphasizeLine(int lineNum)
{
    auto logLine = mSubLog->toParentLine(lineNum);
    mLogEdit->scrollToLine(logLine);

    mTimeLine->highlightNode(logLine);
}

void MainWindow::handleSubLogMarkLine(int line, const QString &text)
{
    mTimeLine->addNode(mSubLog->toParentLine(line), text);
}

void MainWindow::search(bool foward)
{
    auto keyword = mSearchEdit->text();
    if (keyword.isEmpty())
        return;

    QTextDocument::FindFlags flag = QTextDocument::FindFlags();
    if (!foward) {
        flag.setFlag(QTextDocument::FindBackward);
    }

    if (mCaseSensitiveCheckBox->isChecked()) {
        flag.setFlag(QTextDocument::FindCaseSensitively);
    }

    if (!mCurLogEdit->search(keyword, flag)) {
        if (flag.testFlag(QTextDocument::FindBackward)) {
            Toast::instance().show(Toast::INFO, "到达顶部，没有找到匹配项");
        } else {
            Toast::instance().show(Toast::INFO, "到达底部，没有找到匹配项");
        }
    }
}

void MainWindow::doOpenFile(const QString &path)
{    
    bool ret = false;
    bool canceled = BackgroundRunner::instance().exec("打开文件", [&](LongtimeOperation& op){
        ret = mLog.open(path, op);
    });

    if (canceled) {
        return;
    }

    if (!ret) {
        Toast::instance().show(Toast::INFO, "文件打开失败");
        return;
    }

    mLogEdit->setLog(&mLog);

    if (mSubLog){
        delete mSubLog;
        mSubLog = nullptr;
    }
    mSubLogEdit->setLog(nullptr);
    mSubLogDWidget->setVisible(false);

    hasDocSetEnbale();

    QSettings config;
    if (config.value("gotoEOF").toBool()) {
        mLogEdit->scrollToLine(mLog.lineCount());
        Toast::instance().show(Toast::INFO, "已定位到最后一行");
    }

    setWindowTitle("loginsight - " + path);
}

void MainWindow::filter(const QString &text, bool caseSenesitive)
{
    if (mSubLog) {
        delete mSubLog;
    }

    auto canceled = BackgroundRunner::instance().exec(QString("过滤%1").arg(text), [&](LongtimeOperation& op){
        mSubLog = mLog.createSubLog(text, caseSenesitive, op);
    });

    if (canceled)
        return;

    if (mSubLog->lineCount() > 0) {
        mSubLogEdit->setLog(mSubLog);
        mSubLogDWidget->setVisible(true);
        Toast::instance().show(Toast::INFO, QString("一共过滤到%1行").arg(mSubLog->lineCount()));
    } else {
        Toast::instance().show(Toast::INFO, "没有找到匹配项");
        mSubLogEdit->setLog(nullptr);
        mSubLogDWidget->setVisible(false);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
    if ((ev->modifiers() & Qt::CTRL) && (ev->key() == Qt::Key_F)) {
        if (ev->modifiers() & Qt::SHIFT)
            mSearchEdit->setSearchFoward(false);
        else
            mSearchEdit->setSearchFoward(true);
        mSearchEdit->setFocus();
        mSearchEdit->selectAll();
    }
}

void MainWindow::createSubLogDockWidget()
{
    mSubLogEdit = new LogTextEdit();
    mSubLogEdit->setAcceptDrops(false);
    auto logEditBar = new QScrollBar(Qt::Vertical);
    mSubLogEdit->setScrollBar(logEditBar);

    auto container = new QHBoxLayout();
    container->addWidget(mSubLogEdit, 1);
    container->addWidget(logEditBar);
    container->setMargin(0);
    container->setSpacing(0);

    auto dw = new QDockWidget("过滤窗口");
    dw->setFeatures(QDockWidget::DockWidgetClosable);
    auto w = new QWidget();
    w->setLayout(container);
    dw->setWidget(w);
    addDockWidget(Qt::BottomDockWidgetArea, dw);
    ui->menuDWidget->addAction(dw->toggleViewAction());

    dw->setVisible(false);
    mSubLogDWidget=dw;

    connect(dw, &QDockWidget::visibilityChanged, [this](bool visible){
        if (!visible) {
            this->mLogEdit->setFocus();
        }
    });
}

void MainWindow::createTimelineDockWidget()
{
    mTimeLine = new TimeLine();
    auto dw = new QDockWidget("时间线");
    dw->setWidget(mTimeLine);
    dw->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
    dw->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::RightDockWidgetArea, dw);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    ui->menuDWidget->addAction(dw->toggleViewAction());
}

void MainWindow::createToolbar()
{
    auto toolbar = new QToolBar("主工具栏");
    toolbar->setIconSize(QSize(16,16));
    toolbar->setFloatable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    {
        auto action = new QAction(QIcon(":/res/img/open.png"), "");
        action->setToolTip("打开文件");
        connect(action, SIGNAL(triggered()), this, SLOT(handleOpenFile()));
        toolbar->addAction(action);
    }
    {
        auto action = new QAction(QIcon(":/res/img/filter.png"), "");
        action->setToolTip("过滤关键字");
        connect(action, SIGNAL(triggered()), this, SLOT(handleFilter()));
        toolbar->addAction(action);
        mFilterAction = action;
    }
    {
        auto action = new QAction(QIcon(":/res/img/locate.png"), "");
        action->setToolTip("跳转到行");
        connect(action, SIGNAL(triggered()), this, SLOT(handleGotoLine()));
        toolbar->addAction(action);
        mGotoLineAction = action;
    }
    {
        auto action = new QAction(QIcon(":/res/img/left.png"), "");
        action->setToolTip("后退");
        connect(action, SIGNAL(triggered()), this, SLOT(handleNavBackward()));
        toolbar->addAction(action);
        mNavBackAction = action;
    }
    {
        auto action = new QAction(QIcon(":/res/img/right.png"), "");
        action->setToolTip("前进");
        connect(action, SIGNAL(triggered()), this, SLOT(handleNavFoward()));
        toolbar->addAction(action);
        mNavAheadAction = action;
    }
    toolbar->addSeparator();
    {
        auto action = new QAction(QIcon(":/res/img/clipboard.png"), "");
        action->setToolTip("复制时间线到粘贴板");
        connect(action, SIGNAL(triggered()), mTimeLine, SLOT(exportToClipboard()));
        toolbar->addAction(action);
    }
    {
        auto action = new QAction(QIcon(":/res/img/export.png"), "");
        action->setToolTip("导出时间线到图片");
        connect(action, SIGNAL(triggered()), this, SLOT(handleExportTimeLine()));
        toolbar->addAction(action);
    }
    addToolBar(toolbar);
}

void MainWindow::createTagbar()
{
    auto tagWidget = new QWidget;
    auto box = new QHBoxLayout();

    mSearchEdit = new SearchEdit();
    mSearchEdit->setMinimumHeight(26);
    mSearchEdit->setMinimumWidth(150);
    mSearchEdit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed));
    box->addWidget(mSearchEdit);

    mCaseSensitiveCheckBox = new QCheckBox();
    mCaseSensitiveCheckBox->setText("大小写敏感");
    box->addWidget(mCaseSensitiveCheckBox);

    mTagList = new TagListWidget;
    mTagList->setMinimumHeight(26);
    mTagList->setMaximumHeight(26);
    mTagList->addTag("abc", Qt::red);
    mTagList->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    connect(mTagList, &TagListWidget::onTagDeleted, [this](const QString& keyword){
        mCurLogEdit->getHighlighter()->clearQuickHighlight(keyword);
    });
    connect(mTagList, &TagListWidget::onTagColorChanged, [this](const QString& keyword, QColor color){
        mCurLogEdit->getHighlighter()->quickHighlight(keyword, color);
    });
    connect(mTagList, &TagListWidget::requestSearchTag, [this](const QString& keyword){
        mSearchEdit->setText(keyword);
        mCaseSensitiveCheckBox->setChecked(true);
        search(mSearchEdit->isSearchFoward());
    });
    connect(mTagList, &TagListWidget::requestFilterTag, [this](const QString& keyword){
        filter(keyword, true);
    });
    box->addWidget(mTagList);

    box->setMargin(5);
    box->setSpacing(10);
    tagWidget->setLayout(box);
    tagWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    auto toolbar = new QToolBar("tag工具栏");
    toolbar->setAllowedAreas(Qt::TopToolBarArea|Qt::BottomToolBarArea);
    toolbar->setFloatable(false);
    toolbar->addWidget(tagWidget);
    addToolBar(toolbar);
}

void MainWindow::createCenterWidget()
{
    mLogEdit = new LogTextEdit();
    auto logEditBar = new QScrollBar(Qt::Vertical);
    mLogEdit->setScrollBar(logEditBar);

    auto container = new QHBoxLayout();
    container->addWidget(mLogEdit, 1);
    container->addWidget(logEditBar);
    container->setMargin(0);
    container->setSpacing(0);

    auto w = new QWidget();
    w->setLayout(container);

    mCurLogEdit = mLogEdit;
    mCurLogEdit->drawFocused();
    mAddTagConnection = connect(mCurLogEdit->getHighlighter(), &Highlighter::onPatternAdded, [this](HighlightPattern p){
        mTagList->addTag(p.key, p.color);
    });

    setCentralWidget(w);
}
