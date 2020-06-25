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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("loginsight");
    setContextMenuPolicy(Qt::NoContextMenu);

    createCenterWidget();
    bindActions();

    showMaximized();

//    QSettings config;
//    ui->caseSensitivityCheckBox->setChecked(config.value("caseSensitive").toBool());

    noDocSetDisable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bindActions()
{
//    connect(ui->searchEdit, SIGNAL(searchFoward()), this, SLOT(handleSearchFoward()));
//    connect(ui->searchEdit, SIGNAL(searchBackward()), this, SLOT(handleSearchBackward()));

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
//    connect(mLogEdit, SIGNAL(returnPressed()), ui->searchEdit, SLOT(transferReturnBehavior()));
//    connect(mSubLogEdit, SIGNAL(returnPressed()), ui->searchEdit, SLOT(transferReturnBehavior()));
    connect(mLogEdit, &LogTextEdit::beenFocused, this, &MainWindow::handleLogEditFocus);
    connect(mSubLogEdit, &LogTextEdit::beenFocused, this, &MainWindow::handleLogEditFocus);
    connect(mLogEdit, SIGNAL(emphasizeLine(int)), this, SLOT(handleLogEditEmphasizeLine(int)));
    connect(mSubLogEdit, SIGNAL(emphasizeLine(int)), this, SLOT(handleSubLogEditEmphasizeLine(int)));
    connect(mLogEdit, SIGNAL(requestFilter(const QString&)), this, SLOT(handleFilterRequest(const QString&)));
    connect(mSubLogEdit, SIGNAL(requestFilter(const QString&)), this, SLOT(handleFilterRequest(const QString&)));
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

//    //检视
//    connect(ui->actionsearch, &QAction::triggered, [this]{
//        ui->searchEdit->setSearchFoward(true);
//        ui->searchEdit->setFocus();
//    });
//    connect(ui->actionreverseSearch, &QAction::triggered, [this]{
//        ui->searchEdit->setSearchFoward(false);
//        ui->searchEdit->setFocus();
//    });
    connect(ui->actionsearchFoward, &QAction::triggered, this, &MainWindow::handleSearchFoward);
    connect(ui->actionsearchBack, &QAction::triggered, this, &MainWindow::handleSearchFoward);
    connect(ui->actionfilter, &QAction::triggered, this, &MainWindow::handleFilter);
    connect(ui->actiongoBack, &QAction::triggered, this, &MainWindow::handleNavBackward);
    connect(ui->actiongoFoward, &QAction::triggered, this, &MainWindow::handleNavFoward);
    connect(ui->actiongotoLine, &QAction::triggered, this, &MainWindow::handleGotoLine);

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
    connect(ui->actionabout, &QAction::triggered, [this]{
        mAboutDlg.exec();
    });
}

void MainWindow::noDocSetDisable()
{
    ui->actionclose->setEnabled(false);
    ui->menuEncoding->setEnabled(false);
    ui->menuInsight->setEnabled(false);

    ui->actiongoBack->setEnabled(false);
    ui->actiongoFoward->setEnabled(false);
//    ui->navBackAction->setEnabled(false);
//    ui->navAheadAction->setEnabled(false);

//    ui->searchEdit->setEnabled(false);
//    ui->gotoLineAction->setEnabled(false);
//    ui->filterAction->setEnabled(false);
}

void MainWindow::hasDocSetEnbale()
{
    ui->actionclose->setEnabled(true);
    ui->menuEncoding->setEnabled(true);
    ui->menuInsight->setEnabled(true);

    ui->actiongoBack->setEnabled(false);
    ui->actiongoFoward->setEnabled(false);
//    ui->navBackAction->setEnabled(false);
//    ui->navAheadAction->setEnabled(false);

//    ui->searchEdit->setEnabled(true);
//    ui->gotoLineAction->setEnabled(true);
//    ui->filterAction->setEnabled(true);
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
        mSubLogEdit->setVisible(false);
    }

    mLog.close();
    mLogEdit->clear();
    mTimeLine->clear();

    noDocSetDisable();
}

void MainWindow::handleHistoryPosChanged()
{
//    ui->navBackAction->setEnabled(mCurLogEdit->history()->availableBackwardCount() > 0);
    ui->actiongoBack->setEnabled(mCurLogEdit->history()->availableBackwardCount() > 0);
//    ui->navAheadAction->setEnabled(mCurLogEdit->history()->availableFowardCount() > 0);
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
//    auto keyword = ui->searchEdit->text();
//    if (keyword.isEmpty())
//        return;

//    QTextDocument::FindFlags flag = QTextDocument::FindFlags();
//    if (!foward) {
//        flag.setFlag(QTextDocument::FindBackward);
//    }

//    if (ui->caseSensitivityCheckBox->isChecked()) {
//        flag.setFlag(QTextDocument::FindCaseSensitively);
//    }

//    if (!mCurLogEdit->search(keyword, flag)) {
//        if (flag.testFlag(QTextDocument::FindBackward)) {
//            Toast::instance().show(Toast::INFO, "到达顶部，没有找到匹配项");
//        } else {
//            Toast::instance().show(Toast::INFO, "到达底部，没有找到匹配项");
//        }
//    }
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
    mSubLogEdit->setVisible(false);

    hasDocSetEnbale();

    QSettings config;
    if (config.value("gotoEOF").toBool()) {
        mLogEdit->scrollToLine(mLog.lineCount());
        Toast::instance().show(Toast::INFO, "已定位到最后一行");
    }
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
        mSubLogEdit->setVisible(true);
        Toast::instance().show(Toast::INFO, QString("一共过滤到%1行").arg(mSubLog->lineCount()));
    } else {
        Toast::instance().show(Toast::INFO, "没有找到匹配项");
        mSubLogEdit->setLog(nullptr);
        mSubLogEdit->setVisible(false);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
//    if ((ev->modifiers() & Qt::CTRL) && (ev->key() == Qt::Key_F)) {
//        if (ev->modifiers() & Qt::SHIFT)
//            ui->searchEdit->setSearchFoward(false);
//        else
//            ui->searchEdit->setSearchFoward(true);
//        ui->searchEdit->setFocus();
//        ui->searchEdit->selectAll();
//    }
}

void MainWindow::createCenterWidget()
{
    auto* timeLineSplitter = new QSplitter(Qt::Horizontal);

    //log edit
    auto* logSplitter = new QSplitter(Qt::Vertical);

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
        logSplitter->addWidget(w);
    }

    {
        mSubLogEdit = new LogTextEdit();
        auto logEditBar = new QScrollBar(Qt::Vertical);
        mSubLogEdit->setScrollBar(logEditBar);

        auto container = new QHBoxLayout();
        container->addWidget(mSubLogEdit, 1);
        container->addWidget(logEditBar);
        mSubLogEdit->setVisible(false);
        container->setMargin(0);
        container->setSpacing(0);

        auto w = new QWidget();
        w->setLayout(container);
        logSplitter->addWidget(w);
    }

    logSplitter->setStretchFactor(0, 8);
    logSplitter->setStretchFactor(1, 2);

    mCurLogEdit = mLogEdit;
    mCurLogEdit->drawFocused();

    //timeline
    timeLineSplitter->addWidget(logSplitter);
    mTimeLine = new TimeLine();
    timeLineSplitter->addWidget(mTimeLine);

    timeLineSplitter->setStretchFactor(0,12);
    timeLineSplitter->setStretchFactor(0,5);

    //关键词
    auto centerLayout = new QVBoxLayout();
    auto list = new QListWidget();
    list->setMinimumHeight(20);
    list->setMaximumHeight(20);
    centerLayout->addWidget(list);
    centerLayout->addWidget(timeLineSplitter);
    centerLayout->setMargin(3);
    centerLayout->setSpacing(0);
    auto w = new QWidget();
    w->setLayout(centerLayout);
    setCentralWidget(w);

    //toolbar
    auto toolbar = new QToolBar("主工具栏");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(16,16));
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
    }
    {
        auto action = new QAction(QIcon(":/res/img/locate.png"), "");
        action->setToolTip("调整到行");
        connect(action, SIGNAL(triggered()), this, SLOT(handleGotoLine()));
        toolbar->addAction(action);
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
