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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("loginsight");

    ui->timeLineSplitter->setStretchFactor(0, 14);
    ui->timeLineSplitter->setStretchFactor(1, 5);

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

    connect(ui->searchEdit, SIGNAL(searchFoward()), this, SLOT(handleSearchFoward()));
    connect(ui->searchEdit, SIGNAL(searchBackward()), this, SLOT(handleSearchBackward()));

    connect(ui->gotoLineAction, SIGNAL(clicked()), this, SLOT(handleGotoLine()));
    connect(ui->openAction, SIGNAL(clicked()), this, SLOT(handleOpenFile()));

    connect(ui->navBackAction, SIGNAL(clicked()), this, SLOT(handleNavBackward()));
    connect(ui->navAheadAction, SIGNAL(clicked()), this, SLOT(handleNavFoward()));
    ui->navBackAction->setEnabled(false);
    ui->navAheadAction->setEnabled(false);

    connect(ui->clipboardAction, SIGNAL(clicked()), ui->timeLine, SLOT(exportToClipboard()));

    connect(ui->logEdit->history(), SIGNAL(posChanged()), this, SLOT(handleHistoryPosChanged()));
    connect(ui->subLogEdit->history(), SIGNAL(posChanged()), this, SLOT(handleHistoryPosChanged()));
    connect(ui->logEdit, SIGNAL(returnPressed()), ui->searchEdit, SLOT(transferReturnBehavior()));
    connect(ui->subLogEdit, SIGNAL(returnPressed()), ui->searchEdit, SLOT(transferReturnBehavior()));

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
    ui->clipboardAction->setShortcut(QKeySequence::Save);

    //    resize(800,500);
    showMaximized();
}

MainWindow::~MainWindow()
{
    delete ui;
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
    QDialog inputDlg;
    inputDlg.setWindowTitle("过滤日志");
    QVBoxLayout layout(&inputDlg);
    QCheckBox caseSensentiveCheckBox("大小写敏感");
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
    }
}
