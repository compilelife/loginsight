#include "mainwindow.h"
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QTabBar>
#include <QDebug>
#include <QSplitter>
#include "documenttab.h"
#include "filesource.h"
#include "progressdialog.h"
#include <memory>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include "searchbar.h"
#include <QMessageBox>
#include "filterdialog.h"
#include <QInputDialog>
#include <QSettings>
#include <QAction>
#include <memory>
#include "QTextCodec"
#include <QJsonArray>
#include "version.h"
#include <QFileDialog>
#include "settingsdialog.h"
#include <QJsonDocument>
#include <QDesktopServices>
#include <QUrl>
#include "aboutdlg.h"
#include "updater.h"
#include "webhome.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include "feedbackdlg.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildTabWidget();
    mWelcomePage = new WelcomePage(this);

    mCenterWidget = new QStackedWidget;
    mCenterWidget->addWidget(mTabWidget);
    mCenterWidget->addWidget(mWelcomePage);

    setCentralWidget(mCenterWidget);

    mCenterWidget->setCurrentWidget(mWelcomePage);

        buildMenuBar();
        buildToolbars();

        bindUserControls();
        noDocDisableActions();

        this->setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    auto cnt = mTabWidget->count();
    while(cnt-->0) {
        doCloseDocumentTab(0);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    auto data = ev->mimeData();
    if (!data->hasUrls())
        return;
    auto url = data->urls()[0];
    if (!QFileInfo(url.toLocalFile()).isFile())
        return;

    ev->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *ev)
{
    auto url = ev->mimeData()->urls()[0];
    doOpenFile(url.toLocalFile());
}

void MainWindow::openFile()
{
    auto path = QFileDialog::getOpenFileName(this, "打开文件");
    if (path.isEmpty())
        return;

    doOpenFile(path);
}

void MainWindow::closeDocumentTab(int index)
{
    if (index <0 || index >= mTabWidget->count()) {
        return;
    }

    QSettings config;
    int btn = QMessageBox::Ok;
    if (config.value("closeTabPrompt").toBool()) {
        auto tabText = mTabWidget->tabText(index);
        btn = QMessageBox::warning(this,
                                        "关闭日志", QString("确认要关闭标签页【%1】吗？").arg(tabText),
                                        QMessageBox::Ok,
                                        QMessageBox::Cancel);
    }

    if (btn == QMessageBox::Ok) {
        doCloseDocumentTab(index);
    }
}

void MainWindow::savePrj()
{
    auto savepath = QFileDialog::getSaveFileName(this, "保存工程到...", QString(), "*.liprj");
    if (savepath.isEmpty())
        return;
    if (!savepath.endsWith(".liprj")) {
        savepath += ".liprj";
    }
    auto json = saveToJson();

    QJsonDocument doc(json);
    QFile file(savepath);
    if (!file.open(QFile::WriteOnly)) {
         QMessageBox::critical(this, "保存失败", QString("工程文件无法打开：%1").arg(savepath));
         return;
    }

    if (file.write(doc.toJson()) < 0) {
        QMessageBox::critical(this, "保存失败", QString("工程文件无法写入到%1").arg(savepath));
    }

    file.close();

    mRecentPrj.add(savepath);
}

void MainWindow::loadPrj()
{
    auto filepath = QFileDialog::getOpenFileName(this, "打开工程...", QString(), "*.liprj");
    if (filepath.isEmpty())
        return;

    doOpenPrj(filepath);
}

void MainWindow::doOpenFile(const QString &path)
{
    auto tab = new DocumentTab();
    shared_ptr<FileSource> log(new FileSource(tab));
    log->setFileName(path);
    tab->init(log);
    tab->setName(log->getSimpleDesc());

    if (!log->open()) {
        QMessageBox::critical(this, "无法打开文件", QString("%1打开失败").arg(path));
        return;
    }

    appendDocumentTab(tab);

    mRecentFile.add(path);
}

void MainWindow::doOpenPrj(const QString &path)
{
    QFile file(path);

    if (!file.open(QFile::ReadOnly)) {
         QMessageBox::critical(this, "打开失败", QString("工程文件无法打开：%1").arg(path));
         return;
    }

    auto bytes = file.readAll();
    if (bytes.isEmpty()) {
        QMessageBox::critical(this, "打开失败", QString("工程文件为空：%1").arg(path));
        return;
    }

    loadFromJson(QJsonDocument::fromJson(bytes).object());

    mRecentPrj.add(path);
}

void MainWindow::doCloseDocumentTab(int index)
{
    auto tab = (DocumentTab*)mTabWidget->widget(index);
    tab->unInit();
    mTabWidget->removeTab(index);
    delete tab;

    if (mTabWidget->count() == 0) {
        mCenterWidget->setCurrentWidget(mWelcomePage);
        noDocDisableActions();
    }
}

void MainWindow::noDocDisableActions()
{

}

void MainWindow::hasDocEnableActions()
{

}

void MainWindow::loadFromJson(const QJsonObject &o)
{
    if (o.contains("mainEdit")) {
        QMessageBox::critical(this, "打开失败", "不支持旧格式的工程文件");
        return;
    }
    for (auto&& tabObj : o["tabs"].toArray()) {
        auto tab = new DocumentTab();
        if (tab->loadFromJson(tabObj)) {
            appendDocumentTab(tab);
        }
    }
}

QJsonObject MainWindow::saveToJson()
{
    QJsonObject o;
    o["version"] = VERSION;
    QJsonArray tabObjs;
    auto cnt = mTabWidget->count();
    for (auto i = 0; i < cnt; i++) {
        auto tab = (DocumentTab*) mTabWidget->widget(i);
        tabObjs.append(tab->saveToJson());
    }
    o["tabs"] = tabObjs;
    return o;
}

void MainWindow::bindUserControls()
{
    auto a = UserControl::instance();
    connect(a.actionFor(UserControl::OpenFileIntent), &QAction::triggered, this, &MainWindow::openFile);
    connect(a.actionFor(UserControl::CloseTabIntent), &QAction::triggered, [this]{
        auto index = mTabWidget->currentIndex();
        this->closeDocumentTab(index);
    });
    connect(a.actionFor(UserControl::UsageIntent), &QAction::triggered, []{
        QDesktopServices::openUrl(QUrl("http://www.loginsight.top/manual/"));
    });
    connect(a.actionFor(UserControl::ShortcutIntent), &QAction::triggered, []{
        auto shortcuts = UserControl::instance().getShortcutHint();
        QMessageBox::information(nullptr, "快捷键", shortcuts);
    });
}

void MainWindow::buildTabWidget()
{
    mTabWidget = new QTabWidget();
    mTabWidget->setTabsClosable(true);
    connect(mTabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeDocumentTab);
    connect(mTabWidget, &QTabWidget::currentChanged, [this]{
        if (mLastDocument) {
            mLastDocument->disconnectUserControls();
            mLastDocument = nullptr;
        }

        auto doc = currentDocument();
        if (doc) {
            doc->connectUserControls();
            mLastDocument = doc;
        }
    });
}

void MainWindow::buildToolbars()
{
    //<==mainBar==>
    auto bar = new QToolBar;
    bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    bar->setFloatable(false);
    bar->setIconSize(QSize(16,16));
    bar->addAction(UserControl::instance().actionFor(UserControl::OpenFileIntent));
    bar->addAction(UserControl::instance().actionFor(UserControl::LocateLineIntent));
    bar->addAction(UserControl::instance().actionFor(UserControl::FilterIntent));
    bar->addAction(UserControl::instance().actionFor(UserControl::GoBackwardIntent));
    bar->addAction(UserControl::instance().actionFor(UserControl::GoForwardIntent));
    addToolBar(bar);

    //<==timelineBar==>
    auto timelineBar = new QToolBar;
    timelineBar->setAllowedAreas(Qt::TopToolBarArea|Qt::BottomToolBarArea);
    timelineBar->setFloatable(false);
    timelineBar->setIconSize(QSize(16,16));
    timelineBar->addAction(UserControl::instance().actionFor(UserControl::CopyTimeLineIntent));
    timelineBar->addAction(UserControl::instance().actionFor(UserControl::SaveTimeLineIntent));
    addToolBar(timelineBar);

    //<==auxBar==>
    auto auxBar = new QToolBar;
    auxBar->setAllowedAreas(Qt::TopToolBarArea|Qt::BottomToolBarArea);
    auxBar->setFloatable(false);

    auto searchBar = new SearchBar;
    auxBar->addWidget(searchBar);
    UserControl::instance().setSearchBar(searchBar);

    auto taglist = new TagListWidget;
    taglist->setMinimumHeight(26);
    taglist->setMaximumHeight(26);
    taglist->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    auxBar->addWidget(taglist);
    UserControl::instance().setTagList(taglist);

    addToolBar(auxBar);
}

void MainWindow::buildMenuBar()
{
    auto menuBar = new QMenuBar;
    menuBar->addMenu(buildFileMenu());
    menuBar->addMenu(buildInsightMenu());
    menuBar->addMenu(buildTimelineMenu());
    menuBar->addMenu(buildHelpMenu());
    menuBar->addMenu(buildBuyMenu());
    setMenuBar(menuBar);
}

QMenu* MainWindow::buildTimelineMenu()
{
    auto actions = UserControl::instance();

    auto menu = new QMenu("时间线");
    menu->addAction(actions.actionFor(UserControl::CopyTimeLineIntent));
    menu->addAction(actions.actionFor(UserControl::SaveTimeLineIntent));
    menu->addSeparator();
    menu->addAction(actions.actionFor(UserControl::ClearTimeLineIntent));

    return menu;
}

QMenu *MainWindow::buildHelpMenu()
{
    auto menu = new QMenu("帮助");
    auto a = UserControl::instance();
    menu->addAction(a.actionFor(UserControl::ShortcutIntent));
    menu->addAction(a.actionFor(UserControl::UsageIntent));
    menu->addAction("关于", []{
        AboutDlg dlg;
        dlg.exec();
    });
    menu->addAction("反馈", []{
        FeedbackDlg().exec();
    });

    return menu;
}

QMenu* MainWindow::buildFileMenu()
{
    auto menu = new QMenu("文件");

    auto& actions = UserControl::instance();
    menu->addAction(actions.actionFor(UserControl::OpenFileIntent));
    menu->addAction(actions.actionFor(UserControl::CloseTabIntent));
    menu->addAction("打开工程...", this, &MainWindow::loadPrj);
    menu->addAction("保存工程...", this, &MainWindow::savePrj);

    auto codecMenu = new QMenu("编码");
    auto supportedCodec = QTextCodec::availableCodecs();
    for (auto&& name : supportedCodec) {
        codecMenu->addAction(name,[name, this]{
            auto tab = currentDocument();
            if (tab)
                tab->setCodec(name);
        });
    }
    menu->addMenu(codecMenu);

    menu->addSeparator();


    menu->addSeparator();

    auto recentFileMenu = menu->addMenu("最近打开的文件");
    mRecentFile.mount(recentFileMenu, bind(&MainWindow::doOpenFile, this, placeholders::_1));
    auto recentPrjMenu = menu->addMenu("最近打开的工程");
    mRecentPrj.mount(recentPrjMenu, bind(&MainWindow::doOpenPrj, this, placeholders::_1));

    menu->addSeparator();
    menu->addAction("设置", []{
        SettingsDialog dlg;
        dlg.exec();
    });

    menu->addSeparator();
    menu->addAction("退出", this, &QMainWindow::close);

    return menu;
}

QMenu *MainWindow::buildInsightMenu()
{
    auto actions = UserControl::instance();
    auto menu = new QMenu("检视");
    menu->addAction(actions.actionFor(UserControl::FindIntent));
    menu->addAction(actions.actionFor(UserControl::RevertFindIntent));
    menu->addAction(actions.actionFor(UserControl::FindNextIntent));
    menu->addAction(actions.actionFor(UserControl::FindPreviousIntent));
    menu->addAction(actions.actionFor(UserControl::HighlightIntent));
    menu->addSeparator();
    menu->addAction(actions.actionFor(UserControl::FilterIntent));
    menu->addSeparator();
    menu->addAction(actions.actionFor(UserControl::GoBackwardIntent));
    menu->addAction(actions.actionFor(UserControl::GoForwardIntent));
    menu->addAction(actions.actionFor(UserControl::LocateLineIntent));
    return menu;
}

int MainWindow::appendDocumentTab(DocumentTab *tab)
{
    auto index = mTabWidget->addTab(tab, tab->getName());
    mTabWidget->setCurrentIndex(index);

    if (mTabWidget->count() == 1) {
        mCenterWidget->setCurrentWidget(mTabWidget);
        hasDocEnableActions();
    }
    return index;
}

DocumentTab *MainWindow::currentDocument()
{
    return (DocumentTab*)mTabWidget->currentWidget();
}

QMenu* MainWindow::buildBuyMenu()
{
    auto menu = new QMenu("了解专业版");
    auto gotoDownload = []{
        QDesktopServices::openUrl(QUrl(WEB_DOWNLOAD_URL));
    };

    menu->addAction("下载试用", gotoDownload);

    auto gotoWeb = []{
        QDesktopServices::openUrl(QUrl(WEB_PAGE));
    };
    auto intro = new QMenu("特性介绍");
    intro->addAction("多标签页支持", gotoWeb);
    intro->addAction("多过滤窗/嵌套过滤支持", gotoWeb);
    intro->addAction("多线程加速", gotoWeb);

    menu->addMenu(intro);

    return menu;
}
