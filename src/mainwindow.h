#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <log.h>
#include "timenode.h"
#include "logtextedit.h"
#include "shortcuthelpdlg.h"
#include "timeline.h"
#include "searchedit.h"
#include <QCheckBox>
#include "taglistwidget.h"
#include <QJsonObject>
#include "recent.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void bindActions();
    void bindLogEditActions();
    void bindMenuAction();
    void noDocSetDisable();
    void hasDocSetEnbale();
private slots:
    void handleExportTimeLine();
    void handleFilter();
    void handleNodeSelected(TimeNode* node);
    void handleSearchBackward();
    void handleSearchFoward();
    void handleGotoLine();
    void handleOpenFile();
    void handleCloseFile();
    void handleHistoryPosChanged();
    void handleNavBackward();
    void handleNavFoward();
    void handleLogEditFocus(LogTextEdit* logEdit);
    void setEncoding(const QString &text);
    void handleLogEditEmphasizeLine(int lineNum);
    void handleSubLogEditEmphasizeLine(int lineNum);
    void handleSubLogMarkLine(int line, const QString& text);
    void handleSaveProject();
    void handleOpenProject();
    void doFilter(const Filter &filter);
private:
    void search(bool foward);
public:
    void doOpenFile(const QString& path);
    void doOpenProject(const QString& path);
protected:
    void keyReleaseEvent(QKeyEvent* ev) override;
private:
    Ui::MainWindow *ui;
    FileLog mLog;
    SubLog* mSubLog{nullptr};
    QDockWidget* mSubLogDWidget;
    LogTextEdit* mCurLogEdit;
    ShortcutHelpDlg mShortcutHelpDlg;
    QMetaObject::Connection mAddTagConnection;

private:
    void createSubLogDockWidget();
    void createTimelineDockWidget();
    void createToolbar();
    void createTagbar();
    void createCenterWidget();
    void createRecentActions();
private:
    LogTextEdit* mLogEdit;
    LogTextEdit* mSubLogEdit;
    TimeLine* mTimeLine;
    SearchEdit* mSearchEdit;
    QCheckBox* mCaseSensitiveCheckBox;
    QAction* mNavBackAction;
    QAction* mNavAheadAction;
    QAction* mGotoLineAction;
    QAction* mFilterAction;
    TagListWidget* mTagList;
    QJsonObject mProjectData;
    Recent mRecentFile{"recentFile", 5};
    Recent mRecentPrj{"recentPrj", 5};
};
#endif // MAINWINDOW_H
