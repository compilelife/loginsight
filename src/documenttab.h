#ifndef DOCUMENTTAB_H
#define DOCUMENTTAB_H

#include <QWidget>
#include "ilogsource.h"
#include "logviewer.h"
#include <QTableWidget>
#include <QSplitter>
#include "focusmanager.h"
#include <memory>
#include "timeline.h"
#include "progressdialog.h"
#include <QJsonValue>
#include <functional>
#include <list>

using namespace std;

class LoadingPhase {
public:
    bool process(QEvent* ev);
    void addAction(const function<void()>& action);
private:
    enum {
        eWaitSourceReady,
        eProcessPendings,
        eFinish
    } mState {eWaitSourceReady};
    list<function<void()>> mPendingActions;

    bool runFirstAction();
};

class DocumentTab : public QWidget
{
    Q_OBJECT
public:
    explicit DocumentTab();

    void init(shared_ptr<ILogSource> log);
    void unInit();
    void setName(QString name) {mName = name;}
    QString getName() {return mName;}

public:
    void connectUserControls();
    void disconnectUserControls();

public:
    QJsonValue saveToJson();
    void deepSaveLogToJson(shared_ptr<ILog> &log, QJsonArray &o);
    bool loadFromJson(const QJsonValue &o);
    void deepCreateSubLogFromJson(QJsonObject o);

public slots:
    void find(SearchArg arg, bool forward);
    void filter();
    void setCodec(QString name);
    void saveCurSubLog();

protected:
    void customEvent(QEvent* ev) override;

private slots:
    void locateLine();
    void addHighlight();
    void exportTimeLineToImage();

private:
    void doFilter(SearchArg arg);
    void appendSubLog(LogViewer* logViewer, const QString& hint);
    void closeSubLog(int index);
    LogEdit* currentLogEdit();
    LogEdit* findLogEditByLog(const shared_ptr<ILog>& log);
    void loadTagListWithCurrentLogEdit(LogEdit* edit);
    void connectLogEdit(LogEdit* edit);

private:
    void modalLongOp(shared_ptr<LongtimeOperation> op, QString hint);
    void onSubLogCreated(shared_ptr<SubLog> subLog);
    void onFindDone(shared_ptr<ILog> who, SearchResult ret);
    void onLogChange(shared_ptr<ILog> who, EventAction update);
    void onLineEmphasized(shared_ptr<ILog> who, int line, bool isSource);
    void deepHandleLogChange(const shared_ptr<ILog>&log, Range before, Range after);
    void deepEmphasizeLine(const shared_ptr<ILog>& log, int line, const shared_ptr<ILog>& source);

private:
    std::shared_ptr<ILogSource> mLogSource;
    LogViewer* mLogViewer;
    QTabWidget* mSubLogTabWidget;
    QSplitter* mLogSplitter;
    FocusManager mFocusManager;
    QVector<QMetaObject::Connection> mConnections;
    TimeLine* mTimeLine;
    QString mName;
    unique_ptr<LoadingPhase> mLoadingPharse;

};

#endif // DOCUMENTTAB_H
