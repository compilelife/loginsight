#ifndef MEMLOG_H
#define MEMLOG_H

#include "ilog.h"
#include <QVector>

class MemLog : public ILog
{
public:
    MemLog();
    ~MemLog();
    void init(uchar* mem){mMem = mem;}

public:
    QString readLines(int fromLine, int toLine) override;
    Range availRange() override;

public:
    void addLines(const QVector<qint64> &newBreaks);

private:
    void rawRead(qint64 from, qint64 to, QString& out);
    QPair<qint64,qint64> getLineByteRange(int line);

private:
    uchar* mMem{nullptr};
    QVector<qint64> mLineBreaks;
};

#endif // MEMLOG_H
