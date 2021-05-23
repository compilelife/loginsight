#ifndef FILESOURCE_H
#define FILESOURCE_H

#include "ilogsource.h"
#include <QString>
#include <QFile>
#include <thread>
#include <memory>
#include "memlog.h"
#include <QFuture>
#include <QFutureSynchronizer>

using namespace std;

class FileSource: public ILogSource
{
public:
    FileSource(QObject* eventHandler);
    ~FileSource();

    void setFileName(const QString& path);

    bool open() override;
    void close() override;

    shared_ptr<ILog> getLog() override {return mLog;}

public:
    QJsonValue saveToJson() override;
    void loadFromJson(const QJsonValue &o) override;

private:
    void parse();
    void doClose();

private:
    QFile mFile;
    uchar* mMem{nullptr};
    bool mRunning{false};
    QFuture<void> mParseTask;
    shared_ptr<MemLog> mLog;
};

#endif // FILESOURCE_H
