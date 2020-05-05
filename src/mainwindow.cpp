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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("loginsight");

    resize(800,500);
//    showMaximized();

    ui->timeLineSplitter->setStretchFactor(0, 8);
    ui->timeLineSplitter->setStretchFactor(1, 2);

    ui->logSplitter->setStretchFactor(0, 8);
    ui->logSplitter->setStretchFactor(1, 2);

    ui->logEdit->setEnabled(false);
    ui->logEdit->setScrollBar(ui->logEditVBar);
    ui->logEdit->setLog(&mLog);

    ui->subLogEdit->setScrollBar(ui->subLogEditVBar);
    ui->subLogEdit->setVisible(false);

    connect(ui->logEdit, SIGNAL(requestMarkLine(int, const QString&)), ui->timeLine, SLOT(addNode(int,const QString&)));
    connect(ui->subLogEdit, SIGNAL(requestMarkLine(int, const QString&)), ui->timeLine, SLOT(addNode(int,const QString&)));
    connect(ui->timeLine, SIGNAL(nodeSelected(TimeNode*)), this, SLOT(handleNodeSelected(TimeNode*)));

    connect(ui->exportTimeLineAction, SIGNAL(clicked()), this, SLOT(handleExportTimeLine()));
    connect(ui->filterAction, SIGNAL(clicked()), this, SLOT(handleFilter()));

    auto a = ui->searchEdit->addAction(QIcon(":/res/img/up.png"), QLineEdit::TrailingPosition);
    connect(a, SIGNAL(triggered()), this, SLOT(handleSearchBackward()));
    a = ui->searchEdit->addAction(QIcon(":/res/img/down.png"), QLineEdit::TrailingPosition);
    connect(a, SIGNAL(triggered()), this, SLOT(handleSearchFoward()));

    connect(ui->searchEdit, SIGNAL(returnPressed()), this, SLOT(handleSearchFoward()));
    connect(ui->gotoLineAction, SIGNAL(clicked()), this, SLOT(handleGotoLine()));
    connect(ui->openAction, SIGNAL(clicked()), this, SLOT(handleOpenFile()));

    connect(ui->navBackAction, SIGNAL(clicked()), this, SLOT(handleNavBackward()));
    connect(ui->navAheadAction, SIGNAL(clicked()), this, SLOT(handleNavFoward()));
    ui->navBackAction->setEnabled(false);
    ui->navAheadAction->setEnabled(false);

    connect(ui->logEdit->history(), SIGNAL(posChanged()), this, SLOT(handleHistoryPosChanged()));
    connect(ui->subLogEdit->history(), SIGNAL(posChanged()), this, SLOT(handleHistoryPosChanged()));

    mCurLogEdit = ui->logEdit;
    mCurLogEdit->drawFocused();
    connect(ui->logEdit, &LogTextEdit::beenFocused, this, &MainWindow::handleLogEditFocus);
    connect(ui->subLogEdit, &LogTextEdit::beenFocused, this, &MainWindow::handleLogEditFocus);
    ui->subLogEdit->setSlave(true);
    connect(ui->subLogEdit, SIGNAL(requestLocateMaster(int)), this, SLOT(handleLocateMaster(int)));
    connect(ui->logEdit, SIGNAL(requestFilter(const QString&)), this, SLOT(handleFilterRequest(const QString&)));
    connect(ui->subLogEdit, SIGNAL(requestFilter(const QString&)), this, SLOT(handleFilterRequest(const QString&)));

    ui->openAction->setShortcut(QKeySequence::Open);
    ui->gotoLineAction->setShortcut(QKeySequence("Ctrl+G"));
    ui->filterAction->setShortcut(QKeySequence("Ctrl+L"));
    ui->navBackAction->setShortcut(QKeySequence("Ctrl+["));
    ui->navAheadAction->setShortcut(QKeySequence("Ctrl+]"));
    ui->exportTimeLineAction->setShortcut(QKeySequence::Save);

//    doOpenFile("/Users/chenyong/Downloads/gbk.txt");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleExportTimeLine()
{
    auto filename = QFileDialog::getSaveFileName(this, "保存时间线到", QString(), "Images (*.png *.jpg)");
    ui->timeLine->exportToImage(filename);
}

void MainWindow::handleFilter()
{
    QDialog inputDlg;
    inputDlg.setWindowTitle("过滤日志");
    QVBoxLayout layout(&inputDlg);
    QCheckBox caseSensentiveCheckBox("大小写敏感");
    layout.addWidget(&caseSensentiveCheckBox);
    QLineEdit lineEdit;
    lineEdit.setPlaceholderText("请输入要过滤的关键字");
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
    ui->logEdit->setFocus();
    ui->logEdit->scrollToLine(lineNum);
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

void MainWindow::handleHistoryPosChanged()
{
    ui->navBackAction->setEnabled(mCurLogEdit->history()->availableBackwardCount() > 0);
    ui->navAheadAction->setEnabled(mCurLogEdit->history()->availableFowardCount() > 0);
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

void MainWindow::handleLocateMaster(int lineNum)
{
    ui->logEdit->scrollToLine(lineNum);
    handleLogEditFocus(ui->logEdit);
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
            toast("到达顶部，没有找到匹配项");
        } else {
            toast("到达底部，没有找到匹配项");
        }
    }
}

void MainWindow::toast(const QString &text)
{
    QMessageBox::information(this, "toast", text);
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
        toast("文件打开失败");
        return;
    }

    ui->logEdit->setLog(&mLog);
    if (mSubLog){
        delete mSubLog;
        mSubLog = nullptr;
    }
    ui->subLogEdit->setLog(nullptr);
    ui->subLogEdit->setVisible(false);
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
        toast(QString("一共过滤到%1行").arg(mSubLog->lineCount()));
    } else {
        toast("没有找到匹配项");
        ui->subLogEdit->setLog(nullptr);
        ui->subLogEdit->setVisible(false);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{
    if ((ev->modifiers() & Qt::CTRL) && (ev->key() == Qt::Key_F)) {
        ui->searchEdit->setFocus();
    }
}
