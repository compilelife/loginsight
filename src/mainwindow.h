#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <log.h>
#include "timenode.h"
#include "logtextedit.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void handleExportTimeLine();
    void handleFilter();
    void handleNodeSelected(TimeNode* node);
    void handleSearchBackward();
    void handleSearchFoward();
    void handleGotoLine();
    void handleOpenFile();
    void handleHistoryPosChanged();
    void handleNavBackward();
    void handleNavFoward();
    void handleLogEditFocus(LogTextEdit* logEdit);
    void handleLocateMaster(int lineNum);
private:
    void search(bool foward);
    void toast(const QString& text);
    void doOpenFile(const QString& path);
private:
    Ui::MainWindow *ui;
    FileLog mLog;
    SubLog* mSubLog{nullptr};
    LogTextEdit* mCurLogEdit;

};
#endif // MAINWINDOW_H
