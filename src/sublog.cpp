#include "sublog.h"
#include <algorithm>
#include <QProgressDialog>
#include <QEventLoop>

SubLog::SubLog(shared_ptr<ILog> parent, SearchArg arg)
    :ILog("sub")
{
    mSearchArg = arg;
    mParent = parent;
}

SubLog::~SubLog()
{
    qDebug()<<"~SubLog";
}

QString SubLog::readLines(int from, int to)
{
    QString buffer;

     from -= mFromLineNum;
     to -= mFromLineNum;

     //尝试拼接连续的行
     auto contBegin = mLineInParent[from];
     auto contEnd = contBegin;
     for (auto i = from + 1; i <= to; i++) {
         auto line = mLineInParent[i];
         if (line - 1 == contEnd) {
             contEnd = line;
             continue;
         } else {
             buffer.append(mParent->readLines(contBegin, contEnd));
             contBegin = line;
             contEnd = line;
         }
     }
     buffer.append(mParent->readLines(contBegin, contEnd));

     return buffer;
}

Range SubLog::availRange()
{
    return {mFromLineNum, mFromLineNum+mLineInParent.size()-1};
}

int SubLog::fromParentLine(int line)
{
    auto begin = mLineInParent.begin();
    auto end = mLineInParent.end();

    auto it = std::lower_bound(begin, end, line);
    if (it == end)
        return -1;
    if (*it != line)
        return -1;

    return it - begin + mFromLineNum;
}

int SubLog::toParentLine(int line)
{
    return mLineInParent[line - mFromLineNum];
}

SearchArg SubLog::getSearchArg()
{
    return mSearchArg;
}

//这个导致加载过程中filter把主界面卡死了
bool SubLog::onParentRangeChanged(Range before, Range after)
{
    if (mLineInParent.isEmpty())
        return false;

    bool updated = false;

    //删除要被移除的行
    int i = 0;
    while (mLineInParent[i] < after.from) {
        ++i;
    }
    if (i > 0) {
        updated = true;
        mFromLineNum += i;
        while (i-->0) {
            mLineInParent.pop_front();
        }
    }

    //追加其他行
    auto caseSense = mSearchArg.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    QRegExp pattern(mSearchArg.pattern, caseSense);

    qDebug()<<"update to"<<after.to;
    QProgressDialog dlg("过滤结果更新中", "取消", before.to+1, after.to);

    for (i = before.to + 1; i <= after.to; i++) {
        dlg.setValue(i);

        auto line = mParent->readLine(i);

        auto match = mSearchArg.regex ? line.contains(pattern) : line.contains(mSearchArg.pattern, caseSense);
        if (match) {
            updated = true;
            mLineInParent.push_back(i);
        }
    }

    return updated;
}
