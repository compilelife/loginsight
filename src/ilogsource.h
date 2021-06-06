#ifndef ILOGSOURCE_H
#define ILOGSOURCE_H

#include <QObject>
#include <QApplication>
#include "events.h"
#include <QMutex>
#include <QWaitCondition>
#include <QMutexLocker>
#include "longtimeoperation.h"
#include <QJsonObject>
#include <QTextCodec>

class ILog;
class ILogSource: public enable_shared_from_this<ILogSource>
{
public:
    ILogSource(QObject* eventHandler, QString type)
        :mEventHandler(eventHandler), mType(type){
        setCodec("UTF-8");
    }

    virtual ~ILogSource(){}

    virtual bool open() = 0;
    virtual void pause() {
        QMutexLocker l(&mCtrlMutex);
        mPaused = true;
    }
    virtual void resume() {
        QMutexLocker l(&mCtrlMutex);
        mPaused = false;
        qDebug()<<"notify";
        mCtrlCond.notify_one();
    };
    virtual void close() = 0;

    virtual shared_ptr<ILog> getLog() = 0;

    virtual shared_ptr<LongtimeOperation> getLoadProgress(){
        return mLoadOp;
    }

    virtual bool setCodec(QString name) {
        mCodec = QTextCodec::codecForName(name.toStdString().c_str());
        return true;
    }

    QTextCodec* getCodec() {
        return mCodec;
    }

    static ILogSource *createLogSourceFromType(QString type, QObject* eventHandler);

    virtual QString getSimpleDesc() = 0;

public:
    virtual QJsonValue saveToJson();
    virtual void loadFromJson(const QJsonValue &jo);

public:
    void post(BaseEvent* ev) {
        QMutexLocker l(&mCtrlMutex);
        if (mPaused && (EventsType)ev->type() == evLogChange) {
            qDebug()<<"wait";
            mCtrlCond.wait(&mCtrlMutex);
        }

        qApp->postEvent(mEventHandler, ev);
    }

protected:
    shared_ptr<LongtimeOperation> mLoadOp;
    QTextCodec* mCodec;

private:
    QObject* mEventHandler;
    QMutex mCtrlMutex;
    QWaitCondition mCtrlCond;
    bool mPaused{false};
    QString mType;
};

#endif // ILOGSOURCE_H
