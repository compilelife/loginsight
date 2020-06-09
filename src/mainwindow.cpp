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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("loginsight");

    ui->timeLineSplitter->setStretchFactor(0, 12);
    ui->timeLineSplitter->setStretchFactor(1, 5);

    ui->logSplitter->setStretchFactor(0, 8);
    ui->logSplitter->setStretchFactor(1, 2);

    ui->logEdit->setScrollBar(ui->logEditVBar);
    ui->logEdit->setLog(&mLog);

    ui->subLogEdit->setScrollBar(ui->subLogEditVBar);
    ui->subLogEdit->setVisible(false);

    bindActions();

    mCurLogEdit = ui->logEdit;
    mCurLogEdit->drawFocused();

    //    resize(800,500);
    showMaximized();

    ui->caseSensitivityCheckBox->setChecked(true);

    noDocSetDisable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bindActions()
{
    connect(ui->searchEdit, SIGNAL(searchFoward()), this, SLOT(handleSearchFoward()));
    connect(ui->searchEdit, SIGNAL(searchBackward()), this, SLOT(handleSearchBackward()));

    connect(ui->timeLine, SIGNAL(nodeSelected(TimeNode*)), this, SLOT(handleNodeSelected(TimeNode*)));

    bindLogEditActions();
    bindToolbarAction();
    bindMenuAction();
}

void MainWindow::bindLogEditActions()
{
    connect(ui->logEdit, SIGNAL(requestMarkLine(int, const QString&)), ui->timeLine, SLOT(addNode(int,const QString&)));
    connect(ui->subLogEdit, SIGNAL(requestMarkLine(int, const QString&)), this, SLOT(handleSubLogMarkLine(int, const QString&)));
    connect(ui->logEdit->history(), SIGNAL(posChanged()), this, SLOT(handleHistoryPosChanged()));
    connect(ui->subLogEdit->history(), SIGNAL(posChanged()), this, SLOT(handleHistoryPosChanged()));
    connect(ui->logEdit, SIGNAL(returnPressed()), ui->searchEdit, SLOT(transferReturnBehavior()));
    connect(ui->subLogEdit, SIGNAL(returnPressed()), ui->searchEdit, SLOT(transferReturnBehavior()));
    connect(ui->logEdit, &LogTextEdit::beenFocused, this, &MainWindow::handleLogEditFocus);
    connect(ui->subLogEdit, &LogTextEdit::beenFocused, this, &MainWindow::handleLogEditFocus);
    connect(ui->logEdit, SIGNAL(emphasizeLine(int)), this, SLOT(handleLogEditEmphasizeLine(int)));
    connect(ui->subLogEdit, SIGNAL(emphasizeLine(int)), this, SLOT(handleSubLogEditEmphasizeLine(int)));
    connect(ui->logEdit, SIGNAL(requestFilter(const QString&)), this, SLOT(handleFilterRequest(const QString&)));
    connect(ui->subLogEdit, SIGNAL(requestFilter(const QString&)), this, SLOT(handleFilterRequest(const QString&)));
}

void MainWindow::bindToolbarAction()
{
    connect(ui->filterAction, SIGNAL(clicked()), this, SLOT(handleFilter()));
    connect(ui->clipboardAction, SIGNAL(clicked()), ui->timeLine, SLOT(exportToClipboard()));
    connect(ui->navBackAction, SIGNAL(clicked()), this, SLOT(handleNavBackward()));
    connect(ui->navAheadAction, SIGNAL(clicked()), this, SLOT(handleNavFoward()));
    connect(ui->gotoLineAction, SIGNAL(clicked()), this, SLOT(handleGotoLine()));
    connect(ui->openAction, SIGNAL(clicked()), this, SLOT(handleOpenFile()));
    connect(ui->exportTimeLineAction, SIGNAL(clicked()), this, SLOT(handleExportTimeLine()));
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

    //检视
    connect(ui->actionsearch, &QAction::triggered, [this]{
        ui->searchEdit->setSearchFoward(true);
        ui->searchEdit->setFocus();
    });
    connect(ui->actionreverseSearch, &QAction::triggered, [this]{
        ui->searchEdit->setSearchFoward(false);
        ui->searchEdit->setFocus();
    });
    connect(ui->actionsearchFoward, &QAction::triggered, this, &MainWindow::handleSearchFoward);
    connect(ui->actionsearchBack, &QAction::triggered, this, &MainWindow::handleSearchFoward);
    connect(ui->actionfilter, &QAction::triggered, this, &MainWindow::handleFilter);
    connect(ui->actiongoBack, &QAction::triggered, this, &MainWindow::handleNavBackward);
    connect(ui->actiongoFoward, &QAction::triggered, this, &MainWindow::handleNavFoward);
    connect(ui->actiongotoLine, &QAction::triggered, this, &MainWindow::handleGotoLine);

    //时间线
    connect(ui->actiontoClipboard, &QAction::triggered, ui->timeLine, &TimeLine::exportToClipboard);
    connect(ui->actionexport, &QAction::triggered, this, &MainWindow::handleExportTimeLine);
    connect(ui->actionclearAll, &QAction::triggered, ui->timeLine, &TimeLine::clear);

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
    ui->navBackAction->setEnabled(false);
    ui->navAheadAction->setEnabled(false);

    ui->searchEdit->setEnabled(false);
    ui->gotoLineAction->setEnabled(false);
    ui->filterAction->setEnabled(false);
}

void MainWindow::hasDocSetEnbale()
{
    ui->actionclose->setEnabled(true);
    ui->menuEncoding->setEnabled(true);
    ui->menuInsight->setEnabled(true);

    ui->actiongoBack->setEnabled(false);
    ui->actiongoFoward->setEnabled(false);
    ui->navBackAction->setEnabled(false);
    ui->navAheadAction->setEnabled(false);

    ui->searchEdit->setEnabled(true);
    ui->gotoLineAction->setEnabled(true);
    ui->filterAction->setEnabled(true);
}

void MainWindow::handleExportTimeLine()
{
    auto filename = QFileDialog::getSaveFileName(this, "保存时间线到", QString(), "Images (*.png)");
    if (!filename.contains('.')) {
        filename += ".png";
    }
    ui->timeLine->exportToImage(filename);
}

void MainWindow::handleFilter()
{
    if (!mLog.isOpened())
        return;

    QDialog inputDlg;
    inputDlg.setWindowTitle("过滤日志");

    QVBoxLayout layout(&inputDlg);
    QCheckBox caseSensentiveCheckBox("大小写敏感");
    caseSensentiveCheckBox.setChecked(true);
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
    filter(text, true);
}

void MainWindow::handleNodeSelected(TimeNode *node)
{
    auto lineNum = node->data();
    ui->logEdit->scrollToLine(lineNum);
    if (mSubLog) {
        auto subLogLine = mSubLog->fromParentLine(lineNum);
        if (subLogLine > 0)
            ui->subLogEdit->scrollToLine(subLogLine);
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
        ui->subLogEdit->clear();
        ui->subLogEdit->setVisible(false);
    }

    mLog.close();
    ui->logEdit->clear();
    ui->timeLine->clear();

    noDocSetDisable();
}

void MainWindow::handleHistoryPosChanged()
{
    ui->navBackAction->setEnabled(mCurLogEdit->history()->availableBackwardCount() > 0);
    ui->actiongoBack->setEnabled(mCurLogEdit->history()->availableBackwardCount() > 0);
    ui->navAheadAction->setEnabled(mCurLogEdit->history()->availableFowardCount() > 0);
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
        ui->logEdit->refresh();
        if (ui->subLogEdit->isVisible()) {
            ui->subLogEdit->refresh();
        }
    }
}

void MainWindow::handleLogEditEmphasizeLine(int lineNum)
{
    if (mSubLog) {
        auto subLogLine = mSubLog->fromParentLine(lineNum);
        if (subLogLine > 0)
            ui->subLogEdit->scrollToLine(subLogLine);
    }

    ui->timeLine->highlightNode(lineNum);
}

void MainWindow::handleSubLogEditEmphasizeLine(int lineNum)
{
    auto logLine = mSubLog->toParentLine(lineNum);
    ui->logEdit->scrollToLine(logLine);

    ui->timeLine->highlightNode(logLine);
}

void MainWindow::handleSubLogMarkLine(int line, const QString &text)
{
    ui->timeLine->addNode(mSubLog->toParentLine(line), text);
}

void MainWindow::search(bool foward)
{
    auto keyword = ui->searchEdit->text();
    if (keyword.isEmpty())
        return;

    QTextDocument::FindFlags flag = QTextDocument::FindFlags();
    if (!foward) {
        flag.setFlag(QTextDocument::FindBackward);
    }

    if (ui->caseSensitivityCheckBox->isChecked()) {
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

    ui->logEdit->setLog(&mLog);

    if (mSubLog){
        delete mSubLog;
        mSubLog = nullptr;
    }
    ui->subLogEdit->setLog(nullptr);
    ui->subLogEdit->setVisible(false);

    hasDocSetEnbale();
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
        ui->subLogEdit->setLog(mSubLog);
        ui->subLogEdit->setVisible(true);
        Toast::instance().show(Toast::INFO, QString("一共过滤到%1行").arg(mSubLog->lineCount()));
    } else {
        Toast::instance().show(Toast::INFO, "没有找到匹配项");
        ui->subLogEdit->setLog(nullptr);
        ui->subLogEdit->setVisible(false);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
    if ((ev->modifiers() & Qt::CTRL) && (ev->key() == Qt::Key_F)) {
        if (ev->modifiers() & Qt::SHIFT)
            ui->searchEdit->setSearchFoward(false);
        else
            ui->searchEdit->setSearchFoward(true);
        ui->searchEdit->setFocus();
        ui->searchEdit->selectAll();
    }
}
