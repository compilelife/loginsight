#include "filesource.h"
#include <QDebug>
#include <QVector>
#include "events.h"
#include <QPair>
#include <QtConcurrent>
#include <QFileInfo>
#include <QMessageBox>

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

QString FileSource::getSimpleDesc()
{
    return QFileInfo(mFile.fileName()).fileName();
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
        auto size = mFile.size();
        if (size >= 512*1024*1024L) {
            post(new ErrorEvent([]{
                QMessageBox::warning(nullptr, "打开文件", "当前是开源版本，打开超过500M的文件可能速度较慢。\n大文件的多线程加速仅专业版可用");
            }));
        }
        QVector<qint64> lines;
        lines.reserve(210000);//20M按一行100字符，估计21w行

        for (auto i = 0; i < size && mRunning; i++) {
            auto c = mMem[i];
            if (c == 0) {
                mMem[i] = ' ';//修复一些错误的日志，其中包含了二进制的0，这会意外导致字符串中断
            } else if (c == '\n') {
                lines.push_back(i);
                if (lines.size() >= lines.capacity()) {
                    post(new LogChangeEvent(mLog, [this, lines]{mLog->addLines(lines);}));
                    lines.clear();
                }
            }
        }

        if (!lines.isEmpty()) {
            post(new LogChangeEvent(mLog, [this, lines]{mLog->addLines(lines);}));
        }

        mLoadOp->done();
        post(new SourceReadyEvent);
        post(new SourceFinishEvent);
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
