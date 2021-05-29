#include "filesource.h"
#include <QDebug>
#include <QVector>
#include "events.h"
#include <QPair>
#include <QtConcurrent>

FileSource::FileSource(QObject* eventHandler)
    :ILogSource(eventHandler, "file")
{
    mLog.reset(new MemLog);
}

FileSource::~FileSource()
{
    doClose();
}

void FileSource::setFileName(const QString &path)
{
    mFile.setFileName(path);
}

bool FileSource::open()
{
    if (!mFile.open(QFile::ReadOnly)) {
        qCritical()<<"failed to open";
        return false;
    }

    mMem = mFile.map(0, mFile.size(), QFile::MapPrivateOption);
    if (!mMem) {
        qCritical("mmap failed");
        mFile.close();
        return false;
    }

    mLog->init(mMem);
    mLog->setSource(shared_from_this());

    mRunning=true;
    mLoadOp.reset(new LongtimeOperation);
    parse();

    return true;
}

void FileSource::close()
{
    doClose();
}

QJsonValue FileSource::saveToJson()
{
    auto o = ILogSource::saveToJson().toObject();
    o["path"] = mFile.fileName();
    return o;
}

void FileSource::loadFromJson(const QJsonValue &o)
{
    ILogSource::loadFromJson(o);
    setFileName(o.toObject()["path"].toString());
}

void FileSource::parse()
{
    mParseTask = QtConcurrent::run([this]{
        auto chunkSize = 20*1024*1024;//20M一个块，2G，

        QVector<QPair<qint64,qint64>> taskRanges;
        qint64 offset = 0;
        while (offset + chunkSize <= mFile.size()) {
            taskRanges.push_back({offset, offset + chunkSize - 1});
            offset += chunkSize;
        }
        if (offset + chunkSize > mFile.size()) {
            taskRanges.push_back({offset, mFile.size()-1});
        }
        mLoadOp->setProgressMin(0);
        mLoadOp->setProgressMax(taskRanges.size());

        function<QVector<qint64>(QPair<qint64,qint64>)> mapFunc = [this](QPair<qint64,qint64> r) {
            QVector<qint64> lines;
            lines.reserve(210000);//20M按一行100字符，估计21w行，空间不够的话，每次再分配1w行

//            qDebug()<<r;
            auto from = r.first;
            auto to = r.second;

            for (auto i = from; i <= to && mRunning; i++) {
                auto c = mMem[i];
                if (c == 0) {
                    mMem[i] = ' ';
                } else if (c == '\n') {
                    lines.push_back(i);
                    if (lines.size() >= lines.capacity()) {
                        lines.reserve(lines.size() + 10000);
                    }
                }
            }

            mLoadOp->publishProgress();
            return lines;
        };

        auto rets = QtConcurrent::mapped(taskRanges, mapFunc);

        for (auto i = 0; i < taskRanges.size() && mRunning; i++) {
            auto ret = rets.resultAt(i);
//            qDebug()<<"post "<<i;
            post(new LogChangeEvent(mLog, [this, ret]{mLog->addLines(ret);}));
        }

        mLoadOp->done();
        post(new SourceReadyEvent);
    });
}

void FileSource::doClose()
{
    if (mMem) {
        mRunning = false;
        mParseTask.waitForFinished();
        mFile.unmap(mMem);
        mFile.close();
        mMem = nullptr;
    }

    mLog.reset();
}
