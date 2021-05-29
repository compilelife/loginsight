#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "welcomepage.h"
#include "documenttab.h"
#include <QStackedWidget>
#include "usercontrol.h"
#include "recent.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* ev) override;
    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;

private slots:
    void openFile();
    void closeDocumentTab(int index);
    void savePrj();
    void loadPrj();

public slots:
    void doOpenFile(const QString& path);
    void doOpenPrj(const QString& path);

private:
    void doCloseDocumentTab(int index);
    void noDocDisableActions();
    void hasDocEnableActions();

private:
    void loadFromJson(const QJsonObject& o);
    QJsonObject saveToJson();

private:
    void backgroundCheckUpdate();
    void bindUserControls();
    void buildTabWidget();
    void buildToolbars();
    void buildMenuBar();
    QMenu* buildFileMenu();
    QMenu* buildInsightMenu();
    QMenu* buildTimelineMenu();
    QMenu* buildHelpMenu();
    QMenu* buildBuyMenu();

private:
    int appendDocumentTab(DocumentTab* tab, const QString& title);
    DocumentTab* currentDocument();

private:
    DocumentTab* mLastDocument{nullptr};
    QTabWidget* mTabWidget;
    WelcomePage* mWelcomePage;
    QStackedWidget* mCenterWidget;
    Recent mRecentFile{"recentFile", 5};
    Recent mRecentPrj{"recentPrj", 5};
};
#endif // MAINWINDOW_H
