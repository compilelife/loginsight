#include "memlog.h"
#include "ilogsource.h"
#include <QPair>

MemLog::MemLog()
    :ILog("mem")
{
    mLineBreaks.push_back(-1);
}

MemLog::~MemLog()
{
//    qDebug()<<"~MemLog";
}

QByteArray MemLog::readRawLines(int fromLine, int toLine)
{
    if (!mMem)
        return "";

    QByteArray buffer;

    //尝试拼接连续的行（下一行的from是上一行的to+1)，以合并rawRead调用，提高速度
    auto beginLine = getLineByteRange(fromLine);
    auto from = beginLine.first;
    auto to = beginLine.second;

    for (auto i = fromLine+1; i <= toLine; i++) {
        auto line = getLineByteRange(i);
        if (line.first == to + 1) {
            to = line.second;
        } else {
            rawRead(from, to, buffer);
            from = line.first;
            to = line.second;
        }
    }

    rawRead(from, to, buffer);

    return  buffer;
}

Range MemLog::availRange()
{
    return {1, mLineBreaks.size()-1};
}

void MemLog::addLines(const QVector<qint64> &newBreaks)
{
    mLineBreaks.append(newBreaks);//FIXME: 效率优化
}

void MemLog::rawRead(qint64 from, qint64 to, QByteArray &out)
{
    out.append(QByteArray::fromRawData((char*)mMem+from, to-from+1));
}

QPair<qint64,qint64> MemLog::getLineByteRange(int line)
{
    QPair<qint64,qint64> r;
    r.first = mLineBreaks[line - 1]+1;
    r.second = mLineBreaks[line];
    //某些日志（比如Android monkey日志），行尾是\r\r\n
    //将r.to定位到第一个\r上
    if (r.second > 2 &&mMem[r.second-1] == '\r' && mMem[r.second-2] == '\r') {
        r.second -= 2;
    }
    return r;
}
